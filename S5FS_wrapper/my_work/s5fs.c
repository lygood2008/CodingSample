/*
 *  FILE: s5fs.c
 *  AUTHOR: Yan Li
 *  DESCR: S5FS entry points
 *  TIME: 05/04/2014
 */

#include "kernel.h"
#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "proc/kmutex.h"

#include "fs/s5fs/s5fs_subr.h"
#include "fs/s5fs/s5fs.h"
#include "fs/dirent.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/stat.h"

#include "drivers/dev.h"
#include "drivers/blockdev.h"

#include "mm/kmalloc.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/mm.h"
#include "mm/mman.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"

/* Diagnostic/Utility: */
static int s5_check_super(s5_super_t *super);
static int s5fs_check_refcounts(fs_t *fs);

/* fs_t entry points: */
static void s5fs_read_vnode(vnode_t *vnode);
static void s5fs_delete_vnode(vnode_t *vnode);
static int  s5fs_query_vnode(vnode_t *vnode);
static int  s5fs_umount(fs_t *fs);

/* vnode_t entry points: */
static int  s5fs_read(vnode_t *vnode, off_t offset, void *buf, size_t len);
static int  s5fs_write(vnode_t *vnode, off_t offset, const void *buf, size_t len);
static int  s5fs_mmap(vnode_t *file, vmarea_t *vma, mmobj_t **ret);
static int  s5fs_create(vnode_t *vdir, const char *name, size_t namelen,
                        vnode_t **result);
static int  s5fs_mknod(struct vnode *dir, const char *name, size_t namelen,
                       int mode, devid_t devid);
static int  s5fs_lookup(vnode_t *base, const char *name, size_t namelen,
                        vnode_t **result);
static int  s5fs_link(vnode_t *src, vnode_t *dir, const char *name, size_t namelen);
static int  s5fs_unlink(vnode_t *vdir, const char *name, size_t namelen);
static int  s5fs_mkdir(vnode_t *vdir, const char *name, size_t namelen);
static int  s5fs_rmdir(vnode_t *parent, const char *name, size_t namelen);
static int  s5fs_readdir(vnode_t *vnode, int offset, struct dirent *d);
static int  s5fs_stat(vnode_t *vnode, struct stat *ss);
static int  s5fs_release(vnode_t *vnode, file_t *file);
static int  s5fs_fillpage(vnode_t *vnode, off_t offset, void *pagebuf);
static int  s5fs_dirtypage(vnode_t *vnode, off_t offset);
static int  s5fs_cleanpage(vnode_t *vnode, off_t offset, void *pagebuf);

fs_ops_t s5fs_fsops = {
    s5fs_read_vnode,
    s5fs_delete_vnode,
    s5fs_query_vnode,
    s5fs_umount
};

/* vnode operations table for directory files: */
static vnode_ops_t s5fs_dir_vops = {
    .read      = NULL,
    .write     = NULL,
    .mmap      = NULL,
    .create    = s5fs_create,
    .mknod     = s5fs_mknod,
    .lookup    = s5fs_lookup,
    .link      = s5fs_link,
    .unlink    = s5fs_unlink,
    .mkdir     = s5fs_mkdir,
    .rmdir     = s5fs_rmdir,
    .readdir   = s5fs_readdir,
    .stat      = s5fs_stat,
    .acquire   = NULL,
    .release   = NULL,
    .fillpage  = s5fs_fillpage,
    .dirtypage = s5fs_dirtypage,
    .cleanpage = s5fs_cleanpage
};

/* vnode operations table for regular files: */
static vnode_ops_t s5fs_file_vops = {
    .read      = s5fs_read,
    .write     = s5fs_write,
    .mmap      = s5fs_mmap,
    .create    = NULL,
    .mknod     = NULL,
    .lookup    = NULL,
    .link      = NULL,
    .unlink    = NULL,
    .mkdir     = NULL,
    .rmdir     = NULL,
    .readdir   = NULL,
    .stat      = s5fs_stat,
    .acquire   = NULL,
    .release   = NULL,
    .fillpage  = s5fs_fillpage,
    .dirtypage = s5fs_dirtypage,
    .cleanpage = s5fs_cleanpage
};

/*
 * s5fs_mount:
 * Read fs->fs_dev and set fs_op, fs_root, and fs_i.
 *
 * Point fs->fs_i to an s5fs_t*, and initialize it.  Be sure to
 * verify the superblock (using s5_check_super()).  Use vget() to get
 * the root vnode for fs_root.
 *
 * param *fs: the pointer to the file system object
 * return :0 on success, negative on failure
 */
int
s5fs_mount(struct fs *fs)
{
    int num         = 0;
    blockdev_t* dev = NULL;
    s5fs_t* s5      = NULL;
    pframe_t* vp    = NULL;
    
    KASSERT(fs);
    
    if (sscanf(fs->fs_dev, "disk%d", &num) != 1) {
        return -EINVAL;
    }
    
    if (!(dev = blockdev_lookup(MKDEVID(1, num)))) {
        return -EINVAL;
    }
    
    /* allocate and initialize an s5fs_t: */
    s5 = (s5fs_t *)kmalloc(sizeof(s5fs_t));
    
    if (!s5)
        return -ENOMEM;
    
    /*     init s5f_disk: */
    s5->s5f_bdev  = dev;
    
    /*     init s5f_super: */
    pframe_get(S5FS_TO_VMOBJ(s5), S5_SUPER_BLOCK, &vp);
    
    KASSERT(vp);
    
    s5->s5f_super = (s5_super_t *)(vp->pf_addr);
    
    if (s5_check_super(s5->s5f_super)) {
        /* corrupt */
        kfree(s5);
        return -EINVAL;
    }
    
    pframe_pin(vp);
    
    /*     init s5f_mutex: */
    kmutex_init(&s5->s5f_mutex);
    
    /*     init s5f_fs: */
    s5->s5f_fs  = fs;
    
    
    /* Init the members of fs that we (the fs-implementation) are
     * responsible for initializing: */
    fs->fs_i    = s5;
    fs->fs_op   = &s5fs_fsops;
    fs->fs_root = vget(fs, s5->s5f_super->s5s_root_inode);
    
    return 0;
}

/*
 * s5fs_read_vnode:
 * s5fs_read_vnode will be passed a vnode_t*, which will have its vn_fs
 * and vn_vno fields initialized.
 * param *vnode: the pointer to the vnode object
 */
static void
s5fs_read_vnode(vnode_t *vnode)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(vnode->vn_fs != NULL);
    
    kmutex_lock(&vnode->vn_mutex);
    pframe_t* page = NULL;
    
    int ret = pframe_get(S5FS_TO_VMOBJ(FS_TO_S5FS(vnode->vn_fs)),
                         S5_INODE_BLOCK(vnode->vn_vno), &page);
    
    KASSERT(ret == 0);
    KASSERT(page != NULL);
    
    pframe_pin(page);
    s5_inode_t* inode = ((s5_inode_t*)page->pf_addr) +
                        S5_INODE_OFFSET(vnode->vn_vno);
    
    inode->s5_linkcount++;
    s5_dirty_inode(VNODE_TO_S5FS(vnode), inode);
    vnode->vn_i   = inode;
    vnode->vn_len = inode->s5_size;
    
    switch(inode->s5_type)
    {
        case S5_TYPE_DIR:
        {
            vnode->vn_mode  = S_IFDIR;
            vnode->vn_ops   = &s5fs_dir_vops;
            break;
        }
        case S5_TYPE_DATA:
        {
            vnode->vn_mode  = S_IFREG;
            vnode->vn_ops   = &s5fs_file_vops;
            break;
        }
        case S5_TYPE_CHR:
        {
            vnode->vn_mode  = S_IFCHR;
            vnode->vn_ops   = NULL;
            vnode->vn_devid = (devid_t)(inode->s5_indirect_block);
            vnode->vn_cdev  = bytedev_lookup(vnode->vn_devid);
            break;
        }
        case S5_TYPE_BLK:
        {
            vnode->vn_mode  = S_IFBLK;
            vnode->vn_ops   = NULL;
            vnode->vn_devid = (devid_t)(inode->s5_indirect_block);
            vnode->vn_bdev  = blockdev_lookup(vnode->vn_devid);
            break;
        }
        default:
        {
            panic("inode %d has unknown/invalid type %d!!\n",
                  (int)vnode->vn_vno, (int)inode->s5_type);
        }
    }
    
    kmutex_unlock(&vnode->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
}

/*
 * s5fs_delete_vnode:
 * s5fs_delete_vnode is called by vput when the
 * specified vnode_t no longer needs to exist
 * param *vnode: the pointer to the vnode object
 */
static void
s5fs_delete_vnode(vnode_t *vnode)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(vnode->vn_fs != NULL);
    
    /* Lock */
    kmutex_lock(&vnode->vn_mutex);
    
    pframe_t* page = NULL;
    
    int ret = pframe_get(S5FS_TO_VMOBJ(FS_TO_S5FS(vnode->vn_fs)),
                         S5_INODE_BLOCK(vnode->vn_vno), &page);
    
    KASSERT(ret == 0);
    KASSERT(page != NULL);
    
    s5_inode_t* inode = ((s5_inode_t*)page->pf_addr) +
                        S5_INODE_OFFSET(vnode->vn_vno);
    inode->s5_linkcount--;
    s5_dirty_inode(VNODE_TO_S5FS(vnode), inode);
    
    KASSERT(VNODE_TO_S5INODE(vnode) == inode);
    KASSERT(inode->s5_linkcount >= 0);
    
    if (inode->s5_linkcount== 0)
    {
        s5_free_inode(vnode);
    }
    
    pframe_unpin(page);
    
    /* Unlock */
    kmutex_unlock(&vnode->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
}

/*
 * Called by vput when there are no active references to
 * the vnode. If query_vnode returns 0, vput evicts all pages of the vnode
 * from memory so that it can be deleted.
 * param *vnode: the pointer to the vnode object
 * return: 1 if the vnode still exists, 0 if it can be deleted
 */
static int
s5fs_query_vnode(vnode_t *vnode)
{
    dbg(DBG_S5FS, "{\n");
    dbg(DBG_S5FS, "}\n");
    
    return (VNODE_TO_S5INODE(vnode)->s5_linkcount > 1);
}

/*
 * Unmount the filesystem, performing any necessary reference count
 * checks.  
 * param *fs: the pointer to the file system object
 * return: 0 on success, negative number on error.
 */
static int
s5fs_umount(fs_t *fs)
{
    s5fs_t* s5     = (s5fs_t *)fs->fs_i;
    blockdev_t* bd = s5->s5f_bdev;
    pframe_t* sbp  = NULL;
    int ret        = 0;
    
    if (s5fs_check_refcounts(fs)) {
        dbg(DBG_PRINT, "s5fs_umount: WARNING: linkcount corruption "
            "discovered in fs on block device with major %d "
            "and minor %d!!\n", MAJOR(bd->bd_id), MINOR(bd->bd_id));
    }
    if (s5_check_super(s5->s5f_super)) {
        dbg(DBG_PRINT, "s5fs_umount: WARNING: corrupted superblock "
            "discovered on fs on block device with major %d "
            "and minor %d!!\n", MAJOR(bd->bd_id), MINOR(bd->bd_id));
    }
    
    vnode_flush_all(fs);
    
    vput(fs->fs_root);
    
    if (0 > (ret = pframe_get(S5FS_TO_VMOBJ(s5), S5_SUPER_BLOCK, &sbp))) {
        panic("s5fs_umount: failed to pframe_get super block. "
              "This should never happen (the page should already "
              "be resident and pinned, and even if it wasn't, "
              "block device readpage entry point does not "
              "fail.\n");
    }
    
    KASSERT(sbp);
    
    pframe_unpin(sbp);
    
    kfree(s5);
    
    blockdev_flush_all(bd);
    
    return 0;
}

/* 
 * s5fs_read:
 * Simply call s5_read_file; should be in critical section
 * param *vnode: the pointer to the vnode object
 * param offset: offset in the file where you want to read
 * param *buf: the data should be stored here
 * param len: the length of the buffer
 * return: just return the result of s5_read_file
 */
static int
s5fs_read(vnode_t *vnode, off_t offset, void *buf, size_t len)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(buf != NULL);
    int ret = 0;
    kmutex_lock(&vnode->vn_mutex);
    ret = s5_read_file(vnode, offset, buf, len);
    kmutex_unlock(&vnode->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}

/*
 * s5fs_write:
 * Simply call s5_write_file; should be in critical section
 * param *vnode: the pointer to the vnode object
 * param offset: the offset in the file where you want to write
 * param *buf: the source buffer
 * param len: the length of the buffer
 * return: just return the result of s5_write_file
 */
static int
s5fs_write(vnode_t *vnode, off_t offset, const void *buf, size_t len)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(buf != NULL);
    int ret = 0;
    kmutex_lock(&vnode->vn_mutex);
    ret = s5_write_file(vnode, offset, buf, len);
    kmutex_unlock(&vnode->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}

/*
 * Not applicable now.
 */
static int
s5fs_mmap(vnode_t *file, vmarea_t *vma, mmobj_t **ret)
{
    return 0;
}

/*
 * s5fs_create:
 * s5fs_create is called by open_namev(). it should vget() a new vnode,
 * and create an entry for this vnode in 'dir' of the specified name.
 * param *dir:
 * param name: the name string
 * param namelen: the length of the name
 * param **result: pointer to the address of the result vnode
 * return: 0 on success; negative number on a variety of errors
 */
static int
s5fs_create(vnode_t *dir, const char *name, size_t namelen, vnode_t **result)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(dir != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= NAME_LEN-1);
    
    vnode_t* vn = NULL;
    /* Must be non-exist */
    KASSERT(0 != s5fs_lookup(dir, name, namelen, result));
    /* Lock base */
    kmutex_lock(&dir->vn_mutex);
    int ino;
    if ((ino = s5_alloc_inode(dir->vn_fs, S5_TYPE_DATA, 0)) < 0)
    {
        /* Unsuccessfull*/
        kmutex_unlock(&dir->vn_mutex);
        return ino;
    }
    
    /* May block here */
    vn = vget(dir->vn_fs, (ino_t)ino);
    KASSERT(vn->vn_vno == (ino_t)ino);
    KASSERT(vn != NULL);

    int ret = 0;
    if ((ret = s5_link(dir, vn, name, namelen)) < 0)
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&dir->vn_mutex);
        return ret;
    }
    
    KASSERT(VNODE_TO_S5INODE(vn)->s5_linkcount == 2);
    KASSERT(vn->vn_refcount == 1);
    *result = vn;
    kmutex_unlock(&dir->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    
    return 0;
}

/*
 * s5fs_mknod:
 * s5fs_mknod creates a special file for the device specified by
 * 'devid' and an entry for it in 'dir' of the specified name.
 * param dir: the pointer to the 
 * param name: the name string
 * param namelen: the length of the name string
 * param mode: device mode
 * param devid: device id
 * return: 0 on success; negative number on a variety of errors
 */
static int
s5fs_mknod(vnode_t *dir, const char *name, size_t namelen, int mode, devid_t devid)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(S_ISCHR(mode) || S_ISBLK(mode));
    KASSERT(namelen <= S5_NAME_LEN-1);
    KASSERT(name != NULL);
    KASSERT(dir != NULL);
    
    
    vnode_t* vn = NULL;
    /* Must be non-exist */
    vnode_t* result = NULL;
    
    KASSERT(0 != s5fs_lookup(dir, name, namelen, &result));
    
    /* lock */
    kmutex_lock(&dir->vn_mutex);
    int ino = 0;
    if (S_ISCHR(mode))
    {
        if ((ino = s5_alloc_inode(dir->vn_fs, S5_TYPE_CHR, devid)) < 0)
        {
            /* Unsuccessfull*/
            kmutex_unlock(&dir->vn_mutex);
            return ino;
        }
    }else if (S_ISBLK(mode))
    {
        if ((ino = s5_alloc_inode(dir->vn_fs, S5_TYPE_BLK, devid)) < 0)
        {
            /* Unsuccessfull*/
            kmutex_unlock(&dir->vn_mutex);
            return ino;
        }
    }
    else
    {
        panic("Impossible to get here!");
    }
    
    /* May block here */
    vn = vget(dir->vn_fs, (ino_t)ino);
    KASSERT(vn != NULL);
    KASSERT(vn->vn_vno == (ino_t)ino);
    int ret = 0;
    if ((ret = s5_link(dir, vn, name, namelen)) < 0)
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&dir->vn_mutex);
        return ret;
    }
    /* new vnode */
    KASSERT(vn->vn_refcount == 1);
    
    /* May block here */
    vput(vn);
    kmutex_unlock(&dir->vn_mutex);
    dbg(DBG_S5FS, "}\n");
    
    return 0;
}

/*
 * s5fs_lookup:
 * s5fs_lookup sets *result to the vnode in dir with the specified name.
 * param *base: the vnode object of the base directory
 * param *name: name string
 * param namelen: the length of the name
 * param **result: *result points to the vnode in dir with the specified name
 * return: 0 on success; negative number on a variety of errors
 */
int
s5fs_lookup(vnode_t *base, const char *name, size_t namelen, vnode_t **result)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(base != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= S5_NAME_LEN-1);
    
    kmutex_lock(&base->vn_mutex);
    
    int inode_number = 0;
    if ((inode_number = s5_find_dirent(base, name, namelen)) < 0)
    {
        kmutex_unlock(&base->vn_mutex);
        return inode_number;
    }
    /* May block here */
    /* No modification, no need to lock */
    *result = vget(base->vn_fs, inode_number);
    
    kmutex_unlock(&base->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    return 0;
}

/*
 * s5fs_link:
 * s5fs_link sets up a hard link. it links oldvnode into dir with the
 * specified name.
 * param *src:
 * param *dir:
 * param *name: name string
 * param namelen: the length of the name string
 * return: 0 on success; negative number on a variety of errors
 */
static int
s5fs_link(vnode_t *src, vnode_t *dir, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(src != NULL);
    KASSERT(dir != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= S5_NAME_LEN-1);
    KASSERT(src->vn_fs == dir->vn_fs);
    
    vnode_t* vn = NULL;
    /* Non exist */
    KASSERT(s5fs_lookup(dir, name, namelen, &vn) < 0);
    
    kmutex_lock(&dir->vn_mutex);
    kmutex_lock(&src->vn_mutex);
    
    s5_inode_t* inode = (s5_inode_t*)src->vn_i;
    
    int original_link = inode->s5_linkcount;
    
    KASSERT(inode != NULL);
    
    int ret = s5_link(dir, src, name, namelen);
    
    /* Check increment */
    KASSERT(original_link + 1 == inode->s5_linkcount);
    
    kmutex_unlock(&src->vn_mutex);
    kmutex_unlock(&dir->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}

/*
 * s5fs_unlink:
 * s5fs_unlink removes the link to the vnode in dir specified by name
 * param *dir: the parent's vnode
 * param *name: name string
 * param namelen: the length of the name
 * return: 0 on success, and negative number on a variety of errors
 */
static int
s5fs_unlink(vnode_t *dir, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(dir != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= S5_NAME_LEN-1);
    
    kmutex_lock(&dir->vn_mutex);
    
    int ret = 0;
    
    if ((ret = s5_remove_dirent(dir, name, namelen)) < 0)
    {
        kmutex_unlock(&dir->vn_mutex);
        return ret;
    }
    
    kmutex_unlock(&dir->vn_mutex);
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}


/*
 * s5fs_mkdir:
 * s5fs_mkdir creates a directory called name in dir
 * param *dir: the pointer to the vnode object of the parent directory
 * param *name: the name string
 * param namelen: the length of the name
 * return: 0 on success; negative number on a variety of errors
 */
static int
s5fs_mkdir(vnode_t *dir, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(dir != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= NAME_LEN-1);
    
    vnode_t* result = NULL;
    /* Must be non-exist */
    KASSERT(s5fs_lookup(dir, name, namelen, &result) < 0);
    
    /* Lock */
    kmutex_lock(&dir->vn_mutex);
    s5_inode_t* dir_inode = VNODE_TO_S5INODE(dir);
    int parent_link = dir_inode->s5_linkcount;
    int ino = 0;
    /* Allocate an inode */
    if ((ino = s5_alloc_inode(dir->vn_fs, S5_TYPE_DIR, 0)) < 0)
    {
        /* Unsuccess */
        kmutex_unlock(&dir->vn_mutex);
        return ino;
    }
    
    /* May block here */
    vnode_t* vn = vget(dir->vn_fs, ino);
    
    KASSERT(vn != NULL);
    
    int ret = 0;
    ret = s5_link(dir, vn, name, namelen);
    if (ret < 0)
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&dir->vn_mutex);
        return -1;
    }
    
    s5_inode_t* inode = VNODE_TO_S5INODE(vn);
    
    ret = s5_link(vn, vn, ".", 1);
    if (ret < 0)
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&dir->vn_mutex);
        return -1;
    }
    int a = inode->s5_direct_blocks[0];
    ret = s5_link(vn, dir, "..", 2);
    if (ret < 0)
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&dir->vn_mutex);
        return -1;
    }
    int b = inode->s5_direct_blocks[0];
    if (a != b)
    {
        KASSERT(0);
        b = a;
    }
    KASSERT(inode->s5_linkcount == 2); /* one is VFS, one is from the parent */
    /* Not clear */
    KASSERT(parent_link + 1 == dir_inode->s5_linkcount);
    
    /* May block here */
    vput(vn);
    kmutex_unlock(&dir->vn_mutex);
    KASSERT(ret == 0);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}

/*
 * s5fs_rmdir:
 * s5fs_rmdir removes the directory called name from dir. the directory
 * to be removed must be empty (except for . and .. of course).
 * param *parent: the pointer to the parent dir of the name specified
 * param *name: name string
 * param namelen: the length of the name string
 * return: 0 on success; negative numbers on a variety of errors
 */
static int
s5fs_rmdir(vnode_t *parent, const char *name, size_t namelen)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(parent != NULL);
    KASSERT(name != NULL);
    KASSERT(namelen <= NAME_LEN - 1);
    KASSERT((uint32_t)parent->vn_len == VNODE_TO_S5INODE(parent)->s5_size);
    
    kmutex_lock(&parent->vn_mutex);
    
    int inode_number = 0;
    if ((inode_number = s5_find_dirent(parent, name, namelen)) < 0)
    {
        kmutex_unlock(&parent->vn_mutex);
        /* Need vput? */
        return inode_number;
    }
    
    /* May block here */
    vnode_t* vn = vget(parent->vn_fs, inode_number);
    KASSERT(vn != NULL);
    
    if (!S_ISDIR(vn->vn_mode))
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&parent->vn_mutex);
        return -ENOTDIR;
    }
    
    /* Check empty */
    if (VNODE_TO_S5INODE(vn)->s5_size > sizeof(dirent_t)*2)
    {
        vput(vn);
        kmutex_unlock(&parent->vn_mutex);
        return -ENOTEMPTY;
    }
    
    int ret;
    if ((ret = s5_remove_dirent(parent, name, namelen)) < 0)
    {
        /* May block here */
        vput(vn);
        kmutex_unlock(&parent->vn_mutex);
        return ret;
    }
    /* Decrease the linkcount because .. is removed */
    s5_inode_t* parent_inode = VNODE_TO_S5INODE(parent);
    parent_inode->s5_linkcount--;
    s5_dirty_inode(VNODE_TO_S5FS(parent), parent_inode);
    
    /* May block here */
    vput(vn);
    kmutex_unlock(&parent->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}

/*
 * s5fs_readdir:
 * s5fs_readdir reads one directory entry from the dir into the struct
 * dirent. On success, it returns the amount that offset should be
 * increased by to obtain the next directory entry with a
 * subsequent call to readdir. If the end of the file as been
 * reached (offset == file->vn_len), no directory entry will be
 * read and 0 will be returned.
 * param vnode: the pointer to the vnode object
 * param offset: the offset in the directory where you want to read
 * param d: the dirent object that needs to be filled
 * return: the bytes that have been read
 */
static int
s5fs_readdir(vnode_t *vnode, off_t offset, struct dirent *d)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(S_ISDIR(vnode->vn_mode));
    
    s5_dirent_t tmp;
    int bytes = 0;
    if ((bytes  = s5_read_file(vnode, offset,
                               (char*)&tmp, sizeof(s5_dirent_t))) < 0)
    {
        return bytes;
    }
    else if (bytes == 0)
    {
        return bytes;
    }
    
    d->d_ino = tmp.s5d_inode;
    memcpy(d->d_name, tmp.s5d_name, NAME_LEN);
    d->d_off = 0; /* unused */
    
    dbg(DBG_S5FS, "}\n");
    
    return bytes;
}


/*
 * s5fs_stat:
 * s5fs_stat sets the fields in the given buf, filling it with
 * information about file.
 * param vnode: the pointer to the vnode
 * param ss: the pointer to the stat object that needs to be filled
 * return: always return 0 for success
 */
static int
s5fs_stat(vnode_t *vnode, struct stat *ss)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(ss != NULL);

    kmutex_lock(&vnode->vn_mutex);
    memset(ss, 0, sizeof(struct stat));
    
    s5_inode_t* inode = VNODE_TO_S5INODE(vnode);
    ss->st_mode       = vnode->vn_mode;
    ss->st_ino        = vnode->vn_vno;
    ss->st_nlink      = inode->s5_linkcount - 1;
    ss->st_size       = inode->s5_size;
    ss->st_blocks     = s5_inode_blocks(vnode);
    ss->st_blksize    = PAGE_SIZE;
    kmutex_unlock(&vnode->vn_mutex);
    
    dbg(DBG_S5FS, "}\n");
    
    return 0;
}


/*
 * s5fs_fillpage:
 * Read the page of 'vnode' containing 'offset' into the
 * page-aligned and page-sized buffer pointed to by
 * 'pagebuf'.
 * param *vnode: the pointer to the vnode object
 * param offset: the arbitrary offset of that file (bytes offset)
 * param pagebuf: the buffer for storing the data you have read
 * return: number of bytes that have been read, or negative number for errors
 */
static int
s5fs_fillpage(vnode_t *vnode, off_t offset, void *pagebuf)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(pagebuf != NULL);
    
    int disk_number = 0;
    if ((disk_number = s5_seek_to_block(vnode, offset, 0)) < 0)
    {
        return disk_number;
    }
    if (disk_number == 0)
    {
        memset(pagebuf, 0, PAGE_SIZE);
        return 0;
    }
    
    /* If the disk_number is zero, ignore pagebuf */
    int ret = VNODE_TO_S5FS(vnode)->s5f_bdev->bd_ops->read_block(
              VNODE_TO_S5FS(vnode)->s5f_bdev, pagebuf, disk_number, 1);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}


/*
 * s5fs_dirtypage:
 * A hook; an attempt is being made to dirty the page
 * belonging to 'vnode' that contains 'offset'. (If the
 * underlying fs supports sparse blocks/pages, and the page
 * containing this offset is currently sparse, this is
 * where an attempt should be made to allocate a block in
 * the underlying fs for that block/page). Return zero on
 * success and nonzero otherwise.
 * param vnode: the pointer to the vnode object
 * param offset: the arbitrary offset of that file (bytes offset)
 * return: the block number or 0; negative number for failure
 */
static int
s5fs_dirtypage(vnode_t *vnode, off_t offset)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    
    int disk_number = 0;
    int sparse      = 0;
    if ((sparse = s5_seek_to_block(vnode, offset, 0)) < 0)
    {
        return sparse;
        
    }
    else if (sparse > 0)
    {
        /* find the page */
        return 0;
    }
    
    if ((disk_number = s5_seek_to_block(vnode, offset, 1)) < 0)
    {
        return disk_number;
    }
    
    /* Dirty the page */
    
    s5_dirty_inode(VNODE_TO_S5FS(vnode), VNODE_TO_S5INODE(vnode));
    
    dbg(DBG_S5FS, "}\n");
    
    return 0;
}

/*
 * s5fs_cleanpage:
 * Write the contents of the page-aligned and page-sized
 * buffer pointed to by 'pagebuf' to the page of 'vnode'
 * containing 'offset'.
 * param *vnode: the pointer to the vnode object
 * param offset: the arbitrary offset of that file (bytes offset)
 * param pagebuf: the source buffer
 * return: number of bytes that have been written, or negative number for errors
 */
static int
s5fs_cleanpage(vnode_t *vnode, off_t offset, void *pagebuf)
{
    dbg(DBG_S5FS, "{\n");
    
    KASSERT(vnode != NULL);
    KASSERT(pagebuf != NULL);
    
    /* Alloc */
    int disk_number = 0;
    if ((disk_number = s5_seek_to_block(vnode, offset, 0)) < 0)
    {
        return disk_number;/* error */
    }
    /* If the disk_number is zero, ignore pagebuf */
    int ret = VNODE_TO_S5FS(vnode)->s5f_bdev->bd_ops->write_block(
              VNODE_TO_S5FS(vnode)->s5f_bdev, pagebuf, disk_number, 1);
    
    dbg(DBG_S5FS, "}\n");
    
    return ret;
}
/*
 * s5_check_super:
 * verify the superblock.
 * param super: the pointer tot the super block object
 * returns -1 if the superblock is corrupt, 0 if it is OK.
 */
static int
s5_check_super(s5_super_t *super)
{
    if (!(super->s5s_magic == S5_MAGIC
          && (super->s5s_free_inode < super->s5s_num_inodes
              || super->s5s_free_inode == (uint32_t) - 1)
          && super->s5s_root_inode < super->s5s_num_inodes))
        return -1;
    if (super->s5s_version != S5_CURRENT_VERSION) {
        dbg(DBG_PRINT, "Filesystem is version %d; "
            "only version %d is supported.\n",
            super->s5s_version, S5_CURRENT_VERSION);
        return -1;
    }
    return 0;
}

/*
 * calculate_refcounts:
 * recursively check the refcount from the start of the vnode object
 * specified by the user
 * param *counts: the counts array
 * param *vnode: the pointer to the root vnode object
 */
static void
calculate_refcounts(int *counts, vnode_t *vnode)
{
    int ret;
    
    counts[vnode->vn_vno]++;
    dbg(DBG_S5FS, "calculate_refcounts: Incrementing count of inode %d to"
        " %d\n", vnode->vn_vno, counts[vnode->vn_vno]);
    /*
     * We only consider the children of this directory if this is the
     * first time we have seen it.  Otherwise, we would recurse forever.
     */
    if (counts[vnode->vn_vno] == 1 && S_ISDIR(vnode->vn_mode)) {
        int offset = 0;
        struct dirent d;
        vnode_t* child;
        
        while (0 < (ret = s5fs_readdir(vnode, offset, &d))) {
            /*
             * We don't count '.', because we don't increment the
             * refcount for this (an empty directory only has a
             * link count of 1).
             */
            if (0 != strcmp(d.d_name, ".")) {
                child = vget(vnode->vn_fs, d.d_ino);
                calculate_refcounts(counts, child);
                vput(child);
            }
            offset += ret;
        }
        
        KASSERT(ret == 0);
    }
}

/*
 * s5fs_check_refcounts:
 * This will check the refcounts for the filesystem.  It will ensure that that
 * the expected number of refcounts will equal the actual number.  
 * param *fs: the pointer to the file system object
 * return: 0 on success; negative number on failure
 */
int
s5fs_check_refcounts(fs_t *fs)
{
    s5fs_t* s5fs = (s5fs_t *)fs->fs_i;
    int* refcounts;
    int ret = 0;
    uint32_t i;
    
    /* malloc the ref mem */
    refcounts = kmalloc(s5fs->s5f_super->s5s_num_inodes * sizeof(int));
    KASSERT(refcounts);
    memset(refcounts, 0, s5fs->s5f_super->s5s_num_inodes * sizeof(int));
    
    calculate_refcounts(refcounts, fs->fs_root);
    --refcounts[fs->fs_root->vn_vno]; /* the call on the preceding line
                                       * caused this to be incremented
                                       * not because another fs link to
                                       * it was discovered */
    
    dbg(DBG_PRINT, "Checking refcounts of s5fs filesystem on block "
        "device with major %d, minor %d\n",
        MAJOR(s5fs->s5f_bdev->bd_id), MINOR(s5fs->s5f_bdev->bd_id));
    
    for (i = 0; i < s5fs->s5f_super->s5s_num_inodes; i++) {
        vnode_t *vn;
        
        if (!refcounts[i]) continue;
        
        vn = vget(fs, i);
        KASSERT(vn);
        
        if (refcounts[i] != VNODE_TO_S5INODE(vn)->s5_linkcount - 1) {
            dbg(DBG_PRINT, "   Inode %d, expecting %d, found %d\n", i,
                refcounts[i], VNODE_TO_S5INODE(vn)->s5_linkcount - 1);
            ret = -1;
        }
        vput(vn);
    }
    
    dbg(DBG_PRINT, "Refcount check of s5fs filesystem on block "
        "device with major %d, minor %d completed %s.\n",
        MAJOR(s5fs->s5f_bdev->bd_id), MINOR(s5fs->s5f_bdev->bd_id),
        (ret ? "UNSUCCESSFULLY" : "successfully"));
    
    kfree(refcounts);
    return ret;
}
