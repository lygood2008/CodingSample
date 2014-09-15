/*
 *  FILE: s5fs_subr.c
 *  AUTHOR: Yan Li
 *  DESCR: subrotinues of s5fs function calls
 *  TIME: 05/04/2014
 */

#include "kernel.h"
#include "util/debug.h"
#include "mm/kmalloc.h"
#include "globals.h"
#include "proc/sched.h"
#include "proc/kmutex.h"
#include "errno.h"
#include "util/string.h"
#include "util/printf.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "drivers/dev.h"
#include "drivers/blockdev.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/s5fs/s5fs_subr.h"
#include "fs/s5fs/s5fs.h"
#include "mm/mm.h"
#include "mm/page.h"

#define dprintf(...) dbg(DBG_S5FS, __VA_ARGS__)

#define s5_dirty_super(fs)                                           \
do {                                                         \
pframe_t *p;                                         \
int err;                                             \
pframe_get(S5FS_TO_VMOBJ(fs), S5_SUPER_BLOCK, &p);   \
KASSERT(p);                                          \
err = pframe_dirty(p);                               \
KASSERT(!err                                         \
&& "shouldn\'t fail for a page belonging "   \
"to a block device");                        \
} while (0)


static void s5_free_block(s5fs_t *fs, int block);
static int s5_alloc_block(s5fs_t *);


/*
 * s5_seek_to_block:
 * Return the disk-block number for the given seek pointer (aka file
 * position).
 * param vnode: pointer to vnode
 * param seekptr: seek offset
 * alloc: allocate new page or not
 * return: the block number, or negative number on failure
 */
int
s5_seek_to_block(vnode_t *vnode, off_t seekptr, int alloc)
{
    KASSERT(vnode != 0);
    KASSERT(alloc == 1 || alloc == 0);
    KASSERT(seekptr >= 0);
    
    s5_inode_t* inode = VNODE_TO_S5INODE(vnode);
    
    uint32_t block_number = S5_DATA_BLOCK(seekptr);
    
    /* check if the block_number is valid */
    if (block_number >= S5_NIDIRECT_BLOCKS + S5_NDIRECT_BLOCKS)
    {
        return -1;
    }
    if (block_number < S5_NDIRECT_BLOCKS)
    {
        /* sparse block */
        if (inode->s5_direct_blocks[block_number] == 0)
        {
            /* alloc is zero, simply return */
            if (alloc == 0)
            {
                return 0;
            }
            else
            {
                /* alloc a new disk block */
                int block_num = s5_alloc_block(VNODE_TO_S5FS(vnode));
                if (block_num < 0)
                {
                    /* error */
                    return block_num;
                }
                else
                {
                    /* add the new block to inode */
                    inode->s5_direct_blocks[block_number] = block_num;
                    /* dirty the inode */
                    s5_dirty_inode((VNODE_TO_S5FS(vnode)), inode);
                    return block_num;
                }
            }
        }
        else /* already there*/
        {
            return inode->s5_direct_blocks[block_number];
        }
    }
    else
    {
        /* indirect blocks */
        /* if the indirect block is zero, alloc it firstly */
        pframe_t* page = NULL;
        
        if (inode->s5_indirect_block == 0)
        {
            int block_num = 0;
            if ((block_num = s5_alloc_block(FS_TO_S5FS(vnode->vn_fs))) < 0)
            {
                return block_num;
            }
            else
            {
                int ret = pframe_get(S5FS_TO_VMOBJ(FS_TO_S5FS(vnode->vn_fs)),
                                     block_num, &page);
                if (ret < 0 || page == NULL)
                {
                    return ret;
                }
                else
                {
                    pframe_pin(page);
                    memset(page->pf_addr, 0, S5_BLOCK_SIZE);
                    pframe_dirty(page);
                    pframe_unpin(page);
                    inode->s5_indirect_block = block_num;
                    s5_dirty_inode(FS_TO_S5FS(vnode->vn_fs), inode);
                }
            }
        }
        else
        {
            int ret = pframe_get(S5FS_TO_VMOBJ(FS_TO_S5FS(vnode->vn_fs)),
                                 inode->s5_indirect_block, &page);
            if (ret < 0 || page == NULL)
                return ret;
        }
        
        KASSERT(page != NULL);
        
        pframe_pin(page);
        int off = block_number - S5_NDIRECT_BLOCKS;
        int32_t* addr = ((int32_t*)page->pf_addr) + off;
        pframe_unpin(page);
        
        if (*addr == 0)
        {
            if (alloc == 0)
            {
                return 0;
            }
            else
            {
                int block_num = s5_alloc_block(VNODE_TO_S5FS(vnode));
                if (block_num < 0)
                {
                    return block_num;
                }
                else
                {
                    pframe_pin(page);
                    *addr = block_num;
                    pframe_dirty(page);
                    pframe_unpin(page);
                    return block_num;
                }
            }
        }
        else
        {
            /* already there, return */
            return *addr;
        }
    }
}


/*
 * lock_s5:
 * Locks the mutex for the whole file system
 * param *fs: the pointer to the file system object
 */
static void
lock_s5(s5fs_t *fs)
{
    kmutex_lock(&fs->s5f_mutex);
}

/*
 * unlock_s5:
 * Unlocks the mutex for the whole file system
 * param *fs: the pointer to the file system object
 */
static void
unlock_s5(s5fs_t *fs)
{
    kmutex_unlock(&fs->s5f_mutex);
}


/*
 * s5_write_file:
 * write len bytes to the given inode, starting at seek bytes from the
 * beginning of the inode. On success, 
 * param *vnode: the pointer to the vnode object
 * param seek: the seek position
 * param *bytes: the source buffer
 * param len: the length of the source buffer in bytes
 * return: the number of bytes actually written; on failure, return -errno.
 */
int
s5_write_file(vnode_t *vnode, off_t seek, const char *bytes, size_t len)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(bytes != NULL);
    KASSERT(PAGE_SIZE == S5_BLOCK_SIZE);
    KASSERT((uint32_t)vnode->vn_len == VNODE_TO_S5INODE(vnode)->s5_size);
    
    off_t start_pos = seek;
    off_t start_block_offset = S5_DATA_OFFSET(start_pos);
    off_t end_pos          = MIN(seek + len, S5_MAX_FILE_BLOCKS*PAGE_SIZE);
    off_t end_block_offset = S5_DATA_OFFSET(end_pos);
    
    int start_block = S5_DATA_BLOCK(start_pos);
    int end_block   = S5_DATA_BLOCK(end_pos);
    int ret = 0;
    pframe_t* start = NULL;
    ret = pframe_get(&vnode->vn_mmobj, start_block, &start);
    if (ret < 0)
    {
        dbg(DBG_S5FS, "}(error code returend)\n");
        return ret;
    }
    
    pframe_t* end = NULL;
    ret = pframe_get(&vnode->vn_mmobj, end_block, &end);
    if (ret < 0)
    {
        dbg(DBG_S5FS, "}(error code returend)\n");
        return ret;
    }
    uint32_t num_bytes = 0;
    if (start == end)
    {
        pframe_pin(start);
        memcpy((char*)start->pf_addr + start_block_offset, bytes, len);
        
        KASSERT((char*)start->pf_addr + start_block_offset + len ==
                (char*)start->pf_addr + end_block_offset);
        /* dirty the page */
        pframe_dirty(start);
        pframe_unpin(start);
        
        num_bytes = len;
        s5_inode_t* inode = VNODE_TO_S5INODE(vnode );
        inode->s5_size    = MAX(end_pos, vnode->vn_len);
        vnode->vn_len     = inode->s5_size;
        s5_dirty_inode(VNODE_TO_S5FS(vnode),inode);
    }
    else
    {
        /* copy the start block */
        pframe_pin(start);
        memcpy((char*)start->pf_addr + start_block_offset, bytes,
               S5_BLOCK_SIZE - start_block_offset);
        bytes     += S5_BLOCK_SIZE - start_block_offset;
        num_bytes += S5_BLOCK_SIZE - start_block_offset;
        pframe_dirty(start);
        pframe_unpin(start);

        s5_inode_t* inode = VNODE_TO_S5INODE(vnode );
        inode->s5_size    = MAX(start_pos + num_bytes, (uint32_t)vnode->vn_len);
        vnode->vn_len     = inode->s5_size;
        while (1)
        {
            pframe_t* tmp;
            int block_number = S5_DATA_BLOCK(start_pos + num_bytes );
            ret = pframe_get(&vnode->vn_mmobj, block_number, &tmp);
            if (tmp == NULL)
            {
                VNODE_TO_S5INODE(vnode)->s5_size = MAX(start_pos + num_bytes,
                                                       (uint32_t)vnode->vn_len);
                vnode->vn_len = VNODE_TO_S5INODE(vnode)->s5_size;
                 s5_dirty_inode(VNODE_TO_S5FS(vnode),inode);
                return ret;
            }
            if (tmp == end)
            {
                break;
            }
            pframe_pin(tmp);
            memcpy(tmp->pf_addr, bytes, S5_BLOCK_SIZE);
            pframe_dirty(tmp);
            pframe_unpin(tmp);
            bytes     += S5_BLOCK_SIZE;
            num_bytes += S5_BLOCK_SIZE;
        }
        /* copy the last one */
        
        pframe_pin(end);
        memcpy(end->pf_addr, bytes, len - num_bytes);
        num_bytes += len - num_bytes; /* len */
        pframe_dirty(end);
        pframe_unpin(end);
        
        /* add the size */
        inode->s5_size = MAX(end_pos, vnode->vn_len);
        s5_dirty_inode(VNODE_TO_S5FS(vnode),inode);
        vnode->vn_len = inode->s5_size;
    }
    KASSERT((uint32_t)vnode->vn_len == VNODE_TO_S5INODE(vnode)->s5_size);
    dbg(DBG_S5FS, "}\n");
    
    return num_bytes;
}

/*
 * s5_read_file:
 * Read up to len bytes from the given inode, starting at seek bytes
 * from the beginning of the inode. On success, return the number of
 * bytes actually read, or 0 if the end of the file has been reached; on
 * failure, return -errno.
 * param *vnode: the pointer to the vnode object
 * param seek: the seek position
 * param *bytes: the destination buffer
 * param len: the length of the destination buffer in bytes
 * return: the number of bytes actually read; on failure, return -errno.
 */
int
s5_read_file(struct vnode *vnode, off_t seek, char *dest, size_t len)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(dest != NULL);
    KASSERT(PAGE_SIZE == S5_BLOCK_SIZE);
    KASSERT((uint32_t)vnode->vn_len == VNODE_TO_S5INODE(vnode)->s5_size);
    
    if (seek >= vnode->vn_len)
    {
        return 0;/* EOF*/
    }
    
    uint32_t bytes  = 0;
    off_t start_pos = seek;
    int start_block = S5_DATA_BLOCK(start_pos);
    int ret         = 0;
    pframe_t* start = NULL;
    pframe_t* end   = NULL;
    
    ret = pframe_get(&vnode->vn_mmobj, start_block, &start);
    if (ret < 0)
    {
        return ret;
    }
    
    off_t end_pos = MIN(start_pos + (off_t)len, vnode->vn_len);
    int end_block = S5_DATA_BLOCK(end_pos);
    ret = pframe_get(&vnode->vn_mmobj, end_block, &end);
    if (ret < 0)
    {
        return ret;
    }
    
    if (start == end) /* page same */
    {
        pframe_pin(start);
        memcpy(dest, (char*)start->pf_addr + S5_DATA_OFFSET(start_pos),
               end_pos - start_pos);
        pframe_unpin(start);
        bytes = end_pos - start_pos;
    }
    else
    {
        /* copy the start page */
        pframe_pin(start);
        memcpy(dest, (char*)start->pf_addr + S5_DATA_OFFSET(start_pos),
               S5_BLOCK_SIZE-S5_DATA_OFFSET(start_pos));
        pframe_unpin(start);
        dest  += (S5_BLOCK_SIZE - S5_DATA_OFFSET(start_pos));
        bytes += (S5_BLOCK_SIZE - S5_DATA_OFFSET(start_pos));
        /* find next page */
        int off = start_pos + S5_BLOCK_SIZE - S5_DATA_OFFSET(start_pos);
        
        while(1)
        {
            pframe_t* tmp = NULL;
            int block_number = S5_DATA_BLOCK(off);
            ret = pframe_get(&vnode->vn_mmobj, block_number, &tmp);
            if (tmp == NULL)
            {
                dbg(DBG_S5FS, "}(error code returned)\n");
                return ret;
            }
            if (tmp == end)
            {
                break;
            }else
            {
                pframe_pin(tmp);
                memcpy(dest, tmp->pf_addr, S5_BLOCK_SIZE);
                pframe_unpin(tmp);
                dest  += S5_BLOCK_SIZE;/* shift the dest pointer */
                bytes += S5_BLOCK_SIZE;
            }
        }
        pframe_pin(end);
        memcpy(dest, end->pf_addr, len - bytes);
        pframe_unpin(end);
        bytes = len;
        dbg(DBG_S5FS, "}\n");
    }
    
    return bytes;
}

/*
 * s5_alloc_block:
 * Allocate a new disk-block off the block free list and return it. 
 * param *fs: the pointer to the file system object
 * return: the block number, or negative number if no space
 */
static int
s5_alloc_block(s5fs_t *fs)
{
    dbg(DBG_S5FS, "{\n");
    KASSERT(fs != NULL);
    
    lock_s5(fs);
    s5_super_t* super = fs->s5f_super;
    pframe_t* next_free_block = NULL;
    int ret = 0;
    int block_number = 0;
    
    if (super->s5s_nfree == 0) /* no free pages */
    {
        int ret = pframe_get( S5FS_TO_VMOBJ(fs),
                             super->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1],
                             &next_free_block);
        
        if (ret < 0 || next_free_block == NULL)
        {
            /* No more free blocks */
            
            unlock_s5(fs);
            dbg(DBG_S5FS, "}(error code returned)\n");
            return -ENOSPC;
        }
        else
        {
            block_number = super->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1];
            /* copy the next free block into the super block */
            memcpy((void*)(super->s5s_free_blocks), next_free_block->pf_addr,
                   S5_NBLKS_PER_FNODE * sizeof(uint32_t));
            
            memset(next_free_block->pf_addr, 0, S5_BLOCK_SIZE);
            
            pframe_dirty(next_free_block);
            pframe_clean(next_free_block);
            pframe_free(next_free_block);
            /* now full */
            super->s5s_nfree = S5_NBLKS_PER_FNODE-1;
            
            s5_dirty_super(fs);
            unlock_s5(fs);
            dbg(DBG_S5FS, "}\n");
            return block_number;
        }
    }
    else /* there is some free pages */
    {
        block_number = super->s5s_free_blocks[super->s5s_nfree - 1];
        super->s5s_nfree--;
        s5_dirty_super(fs);
        unlock_s5(fs);
        dbg(DBG_S5FS, "}\n");
        return block_number;
    }
}


/*
 * s5_free_block
 * Given a filesystem and a block number, frees the given block in the
 * filesystem.
 *
 * This function may potentially block.
 * param *fs: the pointer to the file system
 * param blockno: the block number
 */
static void
s5_free_block(s5fs_t *fs, int blockno)
{
    s5_super_t *s = fs->s5f_super;
    
    lock_s5(fs);
    
    KASSERT(S5_NBLKS_PER_FNODE > s->s5s_nfree);
    
    if ((S5_NBLKS_PER_FNODE - 1) == s->s5s_nfree)
    {
        /* get the pframe where we will store the free block nums */
        pframe_t *prev_free_blocks = NULL;
        KASSERT(fs->s5f_bdev);
        pframe_get(&fs->s5f_bdev->bd_mmobj, blockno, &prev_free_blocks);
        KASSERT(prev_free_blocks->pf_addr);
        
        /* copy from the superblock to the new block on disk */
        memcpy(prev_free_blocks->pf_addr, (void *)(s->s5s_free_blocks),
               S5_NBLKS_PER_FNODE * sizeof(int));
        pframe_dirty(prev_free_blocks);
        
        /* reset s->s5s_nfree and s->s5s_free_blocks */
        s->s5s_nfree = 0;
        s->s5s_free_blocks[S5_NBLKS_PER_FNODE - 1] = blockno;
    }
    else
    {
        s->s5s_free_blocks[s->s5s_nfree++] = blockno;
    }
    
    s5_dirty_super(fs);
    unlock_s5(fs);
}

/*
 * s5_alloc_inode:
 * Creates a new inode from the free list and initializes its fields.
 * Uses S5_INODE_BLOCK to get the page from which to create the inode
 *
 * This function may block.
 * param *fs: the pointer to the file system
 * param type: the type of this new inode
 * param devid: device id
 * return: the inode number, or -1 for any errors
 */
int
s5_alloc_inode(fs_t *fs, uint16_t type, devid_t devid)
{
    s5fs_t *s5fs      = FS_TO_S5FS(fs);
    pframe_t *inodep  = NULL;
    s5_inode_t *inode = NULL;
    
    int ret = -1;
    
    KASSERT((S5_TYPE_DATA == type)
            || (S5_TYPE_DIR == type)
            || (S5_TYPE_CHR == type)
            || (S5_TYPE_BLK == type));
    
    
    lock_s5(s5fs);
    
    if (s5fs->s5f_super->s5s_free_inode == (uint32_t) - 1)
    {
        unlock_s5(s5fs);
        return -ENOSPC;
    }
    
    pframe_get(&s5fs->s5f_bdev->bd_mmobj,
               S5_INODE_BLOCK(s5fs->s5f_super->s5s_free_inode), &inodep);
    KASSERT(inodep);
    
    inode = (s5_inode_t *)(inodep->pf_addr)
        + S5_INODE_OFFSET(s5fs->s5f_super->s5s_free_inode);
    
    KASSERT(inode->s5_number == s5fs->s5f_super->s5s_free_inode);
    
    ret = inode->s5_number;
    
    /* reset s5s_free_inode; remove the inode from the inode free list: */
    s5fs->s5f_super->s5s_free_inode = inode->s5_next_free;

    pframe_pin(inodep);
    s5_dirty_super(s5fs);
    pframe_unpin(inodep);
    
    
    /* init the newly-allocated inode: */
    inode->s5_size      = 0;
    inode->s5_type      = type;
    inode->s5_linkcount = 0;
    memset(inode->s5_direct_blocks, 0, S5_NDIRECT_BLOCKS * sizeof(int));
    
    if ((S5_TYPE_CHR == type) || (S5_TYPE_BLK == type))
        inode->s5_indirect_block = devid;
    else
        inode->s5_indirect_block = 0;
    
    s5_dirty_inode(s5fs, inode);
    
    unlock_s5(s5fs);
    
    return ret;
}


/*
 * s5_free_inode:
 * Free an inode by freeing its disk blocks and putting it back on the
 * inode free list.
 * param *vnode: the pointer to the vnode object
 */
void
s5_free_inode(vnode_t *vnode)
{
    uint32_t i = 0;
    s5_inode_t *inode = VNODE_TO_S5INODE(vnode);
    s5fs_t *fs = VNODE_TO_S5FS(vnode);
    
    KASSERT((S5_TYPE_DATA == inode->s5_type)
            || (S5_TYPE_DIR == inode->s5_type)
            || (S5_TYPE_CHR == inode->s5_type)
            || (S5_TYPE_BLK == inode->s5_type));
    
    /* free any direct blocks */
    for (i = 0; i < S5_NDIRECT_BLOCKS; ++i)
    {
        if (inode->s5_direct_blocks[i])
        {
            dprintf("freeing block %d\n", inode->s5_direct_blocks[i]);
            s5_free_block(fs, inode->s5_direct_blocks[i]);
            
            s5_dirty_inode(fs, inode);
            inode->s5_direct_blocks[i] = 0;
        }
    }
    
    if (((S5_TYPE_DATA == inode->s5_type)
         || (S5_TYPE_DIR == inode->s5_type))
        && inode->s5_indirect_block)
    {
        pframe_t *ibp;
        uint32_t *b;
        
        pframe_get(S5FS_TO_VMOBJ(fs),
                   (unsigned)inode->s5_indirect_block,
                   &ibp);
        KASSERT(ibp
                && "because never fails for block_device "
                "vm_objects");
        pframe_pin(ibp);
        
        b = (uint32_t *)(ibp->pf_addr);
        for (i = 0; i < S5_NIDIRECT_BLOCKS; ++i)
        {
            KASSERT(b[i] != inode->s5_indirect_block);
            if (b[i])
                s5_free_block(fs, b[i]);
        }
        
        pframe_unpin(ibp);
        
        s5_free_block(fs, inode->s5_indirect_block);
    }
    
    inode->s5_indirect_block = 0;
    inode->s5_type           = S5_TYPE_FREE;
    
    s5_dirty_inode(fs, inode);
    
    lock_s5(fs);
    inode->s5_next_free           = fs->s5f_super->s5s_free_inode;
    fs->s5f_super->s5s_free_inode = inode->s5_number;
    unlock_s5(fs);
    
    s5_dirty_inode(fs, inode);
    s5_dirty_super(fs);
}

/*
 * s5_find_dirent:
 * Locate the directory entry in the given inode with the given name,
 * and return its inode number.
 * return: the inode number, if there is no entry with the given
 * name, return -ENOENT.
 * param *vnode: the pointer to the vnode object
 * param name: name string
 * param namelen: the length of name
 * return: the inode number, or -ENOENT if there is no entry
 */
int
s5_find_dirent(vnode_t *vnode, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(namelen <= S5_NAME_LEN - 1);
    KASSERT(name != NULL);
    KASSERT((uint32_t)vnode->vn_len == VNODE_TO_S5INODE(vnode)->s5_size);
    
    if (!namelen)
    {
        return 0;
    }
    
    s5_dirent_t tmp;
    off_t offset = 0;
    
    while (1)
    {
        int bytes = s5_read_file(vnode, offset, (char*)(&tmp),
                                 sizeof(s5_dirent_t));
        if (bytes < 0)
        {
            /* error */
            dbg(DBG_S5FS, "}(error code returned)\n");
            return bytes;
        }
        else if (bytes == 0)
        {
            dbg(DBG_S5FS, "}(error code returned)\n");
            return -ENOENT;
        }
        else if (bytes != sizeof(s5_dirent_t))
        {
            dbg(DBG_S5FS, "}(error code returned)\n");
            return -1; /* not sure */
        }
        else
        {
            if (name_match(tmp.s5d_name, name, namelen))
            {
                dbg(DBG_S5FS, "}\n");
                return tmp.s5d_inode;
            }
            else
            {
                offset+= sizeof(s5_dirent_t);
            }
        }
    }
    
    KASSERT(0);
}

/*
 * s5_remove_dirent:
 * Locate the directory entry in the given inode with the given name,
 * and delete it. If there is no entry with the given name, return
 * -ENOENT.
 * param *vnode: the pointer to the vnode object
 * param *name: the name string
 * param namelen: the length of the name string
 * return: 0 on success and -ENOENT if there is no entry
 */
int
s5_remove_dirent(vnode_t *vnode, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= S5_NAME_LEN - 1);
    KASSERT((uint32_t)vnode->vn_len == VNODE_TO_S5INODE(vnode)->s5_size);
    KASSERT(vnode->vn_len%sizeof(s5_dirent_t) == 0);
    s5_dirent_t tmp;
    
    off_t offset = 0;
    while (1)
    {
        int bytes = s5_read_file(vnode, offset, (char*)&tmp, sizeof(s5_dirent_t));
        
        if (bytes < 0)
        {
            return bytes;
        }
        else if (bytes == 0)
        {
            return -ENOENT;
        }
        else if (bytes != sizeof(s5_dirent_t))
        {
            return -1;
        }
        else
        {
            if (name_match(tmp.s5d_name, name, namelen))
            {
                if (offset + (off_t)sizeof(s5_dirent_t) < vnode->vn_len)
                {
                    s5_dirent_t last_dirent;
                    int bytes2 = s5_read_file(vnode, vnode->vn_len - sizeof(s5_dirent_t),
                                              (char*)&last_dirent, sizeof(s5_dirent_t));
                    if (bytes2 < 0)
                    {
                        return bytes2;
                    }
                    else if (bytes2 != sizeof(s5_dirent_t))
                    {
                        return -1;
                    }
                    else
                    {
                        bytes2 = s5_write_file(vnode, offset, (char*)&last_dirent,
                                               sizeof(s5_dirent_t));
                        if (bytes2 < 0)
                        {
                            return bytes2;
                        }
                    }
                }
                
                vnode->vn_len -= sizeof(s5_dirent_t);
                s5_inode_t* inode = VNODE_TO_S5INODE(vnode );
                inode->s5_size = vnode->vn_len;
                s5_dirty_inode(VNODE_TO_S5FS(vnode),inode);
                
                vnode_t* vn = vget(vnode->vn_fs,tmp.s5d_inode);
                
                KASSERT(vn);
                s5_inode_t* remove_inode = VNODE_TO_S5INODE(vn);
                remove_inode->s5_linkcount--;
                s5_dirty_inode(FS_TO_S5FS(vn->vn_fs), remove_inode);
                vput(vn);
                
                return 0;
            }
            else
            {
                offset+= sizeof(s5_dirent_t);
            }
        }
    }
    KASSERT(0);
}

/*
 * s5_link:
 * Create a new directory entry in directory 'parent' with the given name, which
 * refers to the same file as 'child'.
 * param *parent: the pointer to the parent vnode object of the vnode child
 * param *child: the pointer to the child vnode object
 * param *name: name string
 * param namelen: the length of the name
 * return: bytes that has been added to the parent's block, or negative number
 *         on failure
 */
int
s5_link(vnode_t *parent, vnode_t *child, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(parent != NULL);
    KASSERT(child != NULL);
    KASSERT(name != NULL);
    KASSERT(S_ISDIR(parent->vn_mode));
    KASSERT((uint32_t)parent->vn_len == VNODE_TO_S5INODE(parent)->s5_size);
    KASSERT((uint32_t)child->vn_len == VNODE_TO_S5INODE(child)->s5_size);
    
    int exist = s5_find_dirent(parent, name, namelen);
    if (exist >= 0)
    {
        return -1;
    }
    else
    {
        s5_inode_t* child_inode = VNODE_TO_S5INODE(child);
        ino_t child_inode_num   = child_inode->s5_number;
        KASSERT(child_inode_num == child->vn_vno);
        
        s5_dirent_t dirent;
        memcpy(dirent.s5d_name, name, S5_NAME_LEN);
        KASSERT(dirent.s5d_name[namelen] == '\0');
        
        dirent.s5d_inode = child_inode_num;
        s5_inode_t* inode_parent = VNODE_TO_S5INODE(parent);
        
        KASSERT((uint32_t)parent->vn_len == inode_parent->s5_size);
        int bytes = s5_write_file(parent, inode_parent->s5_size,
                                  (char*)&dirent, sizeof(s5_dirent_t));
        if (bytes == sizeof(s5_dirent_t))
        {
            /* If not '.' or '..', we need to add the link */
            if (!name_match(name, ".", 1))
            {
                child_inode->s5_linkcount++;
                s5_dirty_inode(VNODE_TO_S5FS(child), child_inode);
            }
            dbg(DBG_S5FS, "}\n");
            return 0;
        }
        return bytes;
    }
}

/*
 * s5_inode_blocks:
 * Return the number of blocks that this inode has allocated on disk.
 * This should include the indirect block, but not include sparse
 * blocks.
 * param *vnode: the pointer to the vnode object
 * return: the number of blocks that this inode has allocated
 */
int
s5_inode_blocks(vnode_t *vnode)
{
    dbg(DBG_S5FS, "{\n");
    KASSERT(vnode != NULL);
    
    s5_inode_t* inode = VNODE_TO_S5INODE(vnode);
    /* Firstly count number of direct blocks */
    int num = 0;
    uint32_t i = 0;
    for (i = 0; i < S5_NDIRECT_BLOCKS; i++)
    {
        if (inode->s5_direct_blocks[i] != 0)
        {
            num++;
        }
    }
    /* Secondly count the number of indirect blocks */
    pframe_t* page = NULL;
    /* May block */
    int ret = pframe_get(S5FS_TO_VMOBJ(FS_TO_S5FS(vnode->vn_fs)),
                         inode->s5_indirect_block, &page);
    KASSERT(ret == 0);
    KASSERT(page != NULL);
    /* Conver to array */
    uint32_t* pageptr = (uint32_t*)(page->pf_addr);
    for (i = 0; i < S5_NIDIRECT_BLOCKS; i++, pageptr++)
    {
        if (*pageptr != 0)
        {
            num++;
        }
    }
    
    dbg(DBG_S5FS, "}\n");
    return num;
}

