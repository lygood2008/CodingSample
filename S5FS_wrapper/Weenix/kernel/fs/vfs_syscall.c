/*
 *  FILE: vfs_syscall.c
 *  AUTH: Yan Li
 *  DESC: vfs interfaces' definition
 *  TIME: 03/26/2014
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/*
 * do_read:
 * To read a file:
 * fget(fd)
 * call its virtual read f_op
 * update f_pos
 * fput() it
 * param fd: the file descriptor
 * param *buf: the buffer you want to read into
 * param nbytes: the number of bytes
 * return: the number of bytes read, or:
 * -EBADF:fd is not a valid file descriptor or is not open for reading.
 * -EISDIR:fd refers to a directory.
 */
int
do_read(int fd, void *buf, size_t nbytes)
{
    if (!(fd >= 0 && fd < NFILES))
        return -EBADF;
    
    file_t* file = NULL;
    if ((file = fget(fd)) == NULL || !FMODE_ISREAD(file->f_mode))
    {
        if (file)
            fput(file);
        
        return -EBADF;
    }
    vnode_t* vnode = file->f_vnode;
    KASSERT(vnode);
    /* The fd is for directory */
    if (S_ISDIR(vnode->vn_mode))
    {
        fput(file);
        return -EISDIR;
    }
    
    vnode_ops_t* ops = vnode->vn_ops;
    
    KASSERT(ops);
    KASSERT(ops->read != NULL);
    int byte_count = ops->read(vnode,file->f_pos, buf, nbytes);
    file->f_pos += byte_count;
    /* release */
    fput(file);
    
    return byte_count;
}

/*
 * do_write:
 * To write a file:
 * fget(fd)
 * call its virtual write f_op
 * update f_pos
 * fput() it
 * param fd: the file descriptor
 * param *buf: the buffer you want to write from
 * param nbytes: the number of bytes
 * return: the number of bytes write, or:
 * -EBADF: fd is not a valid file descriptor or is not open for writing.
 * -EISDIR:fd refers to a directory.
 */
int
do_write(int fd, const void *buf, size_t nbytes)
{
	if (!(fd >= 0 && fd < NFILES))
		return -EBADF;
        
	file_t* file = NULL;

	/* Only read */
	if (((file = fget(fd)) == NULL) ||
	   (FMODE_ISREAD(file->f_mode) && !FMODE_ISWRITE(file->f_mode)))
	{
		if (file)
			fput(file);
		return -EBADF;
	}
        
	/* Append mode, do lseek to the end firstly */
	if (FMODE_ISAPPEND(file->f_mode))
	{
		int ret = 0;
		if ((ret = do_lseek(fd, 0, SEEK_END)) < 0)
		{
			fput(file);
			return ret;
		}
	}

	vnode_t* vnode = file->f_vnode;
	KASSERT(vnode);

	vnode_ops_t* ops = vnode->vn_ops;
	KASSERT(ops);
	KASSERT(ops->write != NULL);

	int bytes_count = ops->write(vnode, file->f_pos, buf, nbytes);
	file->f_pos += bytes_count;
	fput(file);

	return bytes_count;
}

/*
 * do_close:
 * close the file given by the file descriptor
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 * param fd: the file descriptor
 * return: 0 on success or -EBADF on failure (bad file descriptor)
 */
int
do_close(int fd)
{
	file_t* file = NULL;
	if ((!(fd >= 0 && fd < NFILES)) || ((file = fget(fd)) == NULL))
		return -EBADF;
	else
	{
		fput(file);
	}
        
	/* close it*/
	fput(file);
        
	/* zero it*/
	curproc->p_files[fd] = NULL;
        
	/* return zero for success*/
	return 0;
}

/* do_dup:
 * To dup a file:
 * fget(fd) to up fd's refcount
 * get_empty_fd()
 * point the new fd to the same file_t* as the given fd
 * param fd: the file descriptor
 * return: the new file descriptor, or:
 * -EBADF: fd isn't an open file descriptor.
 * -EMFILE: The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int
do_dup(int fd)
{
	file_t* file = NULL;
	if ((!(fd >= 0 && fd < NFILES)) || ((file = fget(fd)) == NULL))
	{
		return -EBADF;
	}
	int id = -1;
	if ((id = get_empty_fd(curproc))< 0)
	{
		fput(file);
		return id;
	}
	else
		curproc->p_files[id] = file;
        
	return id;
}

/*
 * do_dup2:
 * Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first. 
 * param ofd: old file descriptor
 * param nfd: new file descriptor
 * return: the new file descriptor or:
 * -EBADF:ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int
do_dup2(int ofd, int nfd)
{
	/* check range*/
	if (!(ofd >= 0 && ofd < NFILES) || !(nfd >= 0 && nfd < NFILES))
		return -EBADF;
        
	file_t* file = NULL;
	if ((file = fget(ofd)) == NULL)
	{
		return -EBADF;
	}
	KASSERT(file == curproc->p_files[ofd]);
	/* In use */
	if (curproc->p_files[nfd] != NULL)
	{
		if (nfd != ofd)
		{
			do_close(nfd);
		}
		else
		{
			/* dup to itself??? No */
			fput(file);
			return ofd;
		}
	}
	/* new one puts to old one*/
	curproc->p_files[nfd] = curproc->p_files[ofd];
	return nfd;
}

/*
 * do_mknod:
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'.
 * param path: the name string
 * param mode: should only be character or block device
 * param devid: device id
 * return: what mknods return, errors could be:
 * -EINVAL:mode requested creation of something other than a device special
 *        file.
 * -EEXIST:path already exists.
 * -ENOENT:a directory component in path does not exist.
 * -ENOTDIR:a component used as a directory in path is not, in fact, a directory.
 * -ENAMETOOLONG:a component of path was too long.
 */
int
do_mknod(const char *path, int mode, unsigned devid)
{
	if (!S_ISCHR(mode) && !S_ISBLK(mode))
		return -EINVAL;
    
	size_t name_len = 0;
	char name[NAME_LEN + 1] = {0};
	memset(name, 0, sizeof(name));
	char* nameptr = name;
	int ret = 0;
	vnode_t* dir = NULL;
    
	if ((ret = dir_namev(path, &name_len, &nameptr, NULL, &dir)) < 0)
	{
		if (dir)
		{
			vput(dir);
		}
		return ret; /* Implicit errors includes ENAMETOOLONG, ENOTDIR */
	}
	vnode_t* target = NULL;
	/* lookup */
	ret = lookup(dir, name, name_len, &target);
	if (ret == 0)
	{
		KASSERT(target);
		/* already exist */
		vput(target);
		vput(dir);
		return -EEXIST;
	}
    else if (ret != -ENOENT)
	{
		/* Bad thing happens in lookup */
		if (target)
			vput(target);
		vput(dir);
		return ret;
	}
    else
	{
		/* ret is -ENOENT*/
		KASSERT(!target);
		vnode_ops_t* ops = dir->vn_ops;
		KASSERT(ops);
		KASSERT(ops->mknod);
		int ret2 = 0;
		ret2 = ops->mknod(dir, name, name_len, mode, devid);
           
		if (target)
			vput(target);
		vput(dir);
		return ret2;
	}
}

/*
 * do_mkdir:
 * Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. 
 * param *path: the path name string
 * return: what mkdir returns, errors could be:
 * -EEXIST: path already exists.
 * -ENOENT: a directory component in path does not exist.
 * -ENOTDIR: a component used as a directory in path is not, in fact, a directory.
 * -ENAMETOOLONG:a component of path was too long.
 */
int
do_mkdir(const char *path)
{
    /* Empty path */
    if (path == NULL || strlen(path) == 0)
        return -ENOENT;
    size_t name_len = 0;
    char name[NAME_LEN + 1] = {0};
    memset(name, 0, sizeof(name));
    char* nameptr = name;
    vnode_t* dir = NULL;
    int ret = 0;
    if ((ret = dir_namev(path, &name_len, &nameptr, NULL, &dir)) < 0)
    {
        if (dir)
        {
            vput(dir);
        }
        return ret;
    }
    vnode_t* target = NULL;
    ret = lookup(dir, nameptr, name_len, &target);
    
    if (ret == -ENOENT) /* Not exist */
    {
        KASSERT(!target);
        vnode_ops_t* ops = dir->vn_ops;
        KASSERT(ops);
        KASSERT(ops->mkdir != NULL);
        ret = ops->mkdir(dir, name, name_len);

        /* Need to do this? */
        /*if (target)
            vput(target);*/
        vput(dir);
        return ret;
    }
    else if (ret == 0)
    {
        /* Already there */
        KASSERT(target);
        vput(target);
        vput(dir);
        return -EEXIST;
    }
    else
    {
        /* Something bad happens in lookup */
        if (target)
            vput(target);
        vput(dir);
        return ret;
	}
}

/* 
 * do_rmdir:
 * Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.
 * return: 0 on success or:
 * -EINVAL:path has "." as its final component.
 * -ENOTEMPTY:path has ".." as its final component.
 * -ENOENT:a directory component in path does not exist.
 * -ENOTDIR: a component used as a directory in path is not, in fact, a directory.
 * -ENAMETOOLONG:-a component of path was too long.
 */
int
do_rmdir(const char *path)
{
        /* Empty path */
    if (path == NULL || strlen(path) == 0)
    {
        return -ENOENT;
    }
    size_t name_len = 0;
    vnode_t* dir = NULL;
    char name[NAME_LEN + 1] = {0};
    memset(name, 0, sizeof(name));
    char* nameptr = name;
    int ret = 0;
    if ((ret = dir_namev(path, &name_len, &nameptr, NULL, &dir)) < 0)
    {
        if (dir)
            vput(dir);
        return ret; 
    }
    else
    {
        if (!strncmp(name,".",1) && strlen(name) == 1)
        {
            vput(dir);
            return -EINVAL;
        }
        else if (!strncmp(name, "..",2) && strlen(name) == 2)
        {
            vput(dir);
            return -ENOTEMPTY;
        }
        vnode_ops_t* ops = dir->vn_ops;
        KASSERT(ops != NULL);
        KASSERT(ops->rmdir != NULL);
        ret = ops->rmdir(dir, name, name_len);
        vput(dir);
        return ret;
    }
}

/*
 * do_unlink:
 * use dir_namev() to find the vnode of the directory containing the file to be
 * removed.
 * return: 0 on success, or:
 * -EISDIR:path refers to a directory.
 * -ENOENT: a component in path does not exist.
 * -ENOTDIR: a component used as a directory in path is not, in fact, a directory.
 * -ENAMETOOLONG: a component of path was too long.
 */
int
do_unlink(const char *path)
{
    /* Empty path */
    if (path == NULL || strlen(path) == 0)
    {
        return -ENOENT;
    }
    
    size_t name_len = 0;
    vnode_t* dir = NULL;
    char name[NAME_LEN + 1] = {0};
    memset(name, 0, sizeof(name));
    char* nameptr = name;
    int ret = 0;
    
    if ((ret = dir_namev(path, &name_len, &nameptr, NULL, &dir)) < 0)
    {
        if (dir)
            vput(dir);
        return ret;
    }
    else
    {
        /* Need to make sure it does exist */
        /* In ramfs it seems doesn't handle if the file is not there */
        int ret2 = 0;
        vnode_t* target = NULL;
        if ((ret2 = lookup(dir, name, name_len, &target)) < 0) 
        {
            if (target)
                vput(target);
            if (dir)
                vput(dir);
            return ret2;
        }
        else if (S_ISDIR(target->vn_mode))
        {
            vput(target);
            vput(dir);
            return -EISDIR;
        }
        else
        {
            vnode_ops_t* ops = dir->vn_ops;
            KASSERT(ops != NULL);
            KASSERT(ops->unlink);
            int ret3 = 0;
            ret3 = ops->unlink(dir, name, name_len);
            vput(dir);
            vput(target);
            return ret3;
        }
    }
}

/*
 * do_link:
 * open_namev(from)
 * dir_namev(to)
 * call the destination dir's (to) link vn_ops.
 * return the result of link, or an error
 * return 0 on success, or:
 * -EEXIST:to already exists.
 * -ENOENT:A directory component in from or to does not exist.
 * -ENOTDIR:A component used as a directory in from or to is not, in fact, a
 *        directory.
 * -ENAMETOOLONG:A component of from or to was too long.
 */
int
do_link(const char *from, const char *to)
{
	/* Empty path*/
    if ( from == NULL || strlen(from) == 0 || to == NULL || strlen(to) == 0)
        return -ENOENT;

    vnode_t* from_node = NULL;
    int ret = 0;
    if ((ret = open_namev(from, O_RDONLY, &from_node, NULL)) < 0)
    {
        if (from_node)
        {
            vput(from_node);
        }
        return ret;
    }
    else if (!S_ISDIR(from_node->vn_mode))
    {
        vput(from_node);
        return -ENOTDIR;
    }

    vnode_t* to_node = NULL;
    size_t name_len = 0;
    char name[NAME_LEN + 1] = {0};
    memset(name, 0, sizeof(name));
    char* nameptr = name;
    if ((ret = dir_namev(to, &name_len, &nameptr, NULL, &to_node)) < 0)
    {
        vput(from_node);
        if (to_node)
        {
            vput(to_node);
        }
        return ret;
    }

    vnode_t* tmp_node = NULL;
    if ((ret = lookup(to_node, nameptr, name_len, &tmp_node)) < 0)
    {
        if (ret == ENOENT)
        {
            int ret2 = 0;
			vnode_ops_t* ops = from_node->vn_ops;
			KASSERT(ops != NULL);
			ret2 = ops->link(from_node, to_node, name, name_len);
			vput(from_node);
			vput(to_node);
			return ret2;
        }
        else
        {
            /* Something bad happens in lookup */
			vput(from_node);
			vput(to_node);
			if (tmp_node)
				vput(tmp_node);
			return ret;
        }
    }
    else if(ret == 0)
    {
        vput(from_node);
        vput(to_node);
        if (tmp_node)
            vput(tmp_node);
        return -EEXIST;
    }
    KASSERT(0);
    return 0;
}

/*
 * do_rename:
 * link newname to oldname
 * unlink oldname
 * return the value of unlink, or an error
 * param *oldname: name string of old name
 * param *newname: name string of new name
 * return: 0 on success, or -errno on failure
 */
int
do_rename(const char *oldname, const char *newname)
{
	int ret = 0;
	/* link the new name to old name*/
	if ((ret = do_link(oldname, newname)) < 0)
	{
		return ret;
	}
	else
	{
		ret = do_unlink(oldname);
		return ret;
	}
}

/* 
 * do_chdir:
 * Make the named directory the current process's cwd (current working
 * directory).
 * return: 0 on success, or
 * -ENOENT: path does not exist.
 * -ENAMETOOLONG: a component of path was too long.
 * -ENOTDIR: a component of path is not a directory.
 */
int
do_chdir(const char *path)
{
	if (path == NULL || strlen(path) == 0)
        return -ENOENT;
    
    int ret = 0;
    vnode_t* target = NULL;
    if ((ret = open_namev(path, O_RDONLY, &target, NULL)) < 0)
    {
        if (target)
        {
            vput(target);
        }
        return ret; /* Implicit errors inlcude: ENOENT, ENAMETOOLONG */
    }
    else if (!S_ISDIR(target->vn_mode))
    {
        vput(target);
        return -ENOTDIR;
    }
    else
    {
        vnode_t* old = curproc->p_cwd;
        curproc->p_cwd = target;
        vput(old);
        return 0;
    }
}

/* 
 * do_getdent:
 * Call the readdir f_op on the given fd, filling in the given dirent_t*.
 * If the readdir f_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.
 * param fd: the file descriptor
 * param *dirp: the dirent object that you want to read into
 * return: either 0 or sizeof(dirent_t), or -errno.
 */
int
do_getdent(int fd, struct dirent *dirp)
{
	file_t* file = NULL;
	if ((!(fd >= 0 && fd < NFILES)) || ((file = fget(fd)) == NULL))
		return -EBADF;
        
        
	KASSERT(file);
	vnode_t* node = file->f_vnode;
	KASSERT(node);
    
	if (!S_ISDIR(node->vn_mode))
	{
		fput(file);
		return -ENOTDIR;
	}
        
	vnode_ops_t* ops = node->vn_ops;
	KASSERT(ops);
	KASSERT(ops->readdir);

	off_t offset = file->f_pos;
	int ret = 0;
	ret = ops->readdir(node, offset, dirp);
	file->f_pos += ret;
	fput(file);
	size_t sz = sizeof(dirent_t);

	return ret == 0 ? ret : (int)sz;
}

/*
 * do_lseek:
 * Modify f_pos according to offset and whence.
 * param fd: the file descriptor
 * param offset: the offset you want to seek to
 * param whence: seek options
 * return: 0 for success or:
 * -EBADF:fd is not an open file descriptor.
 * -EINVAL:whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int
do_lseek(int fd, int offset, int whence)
{
	if (!(fd >= 0 && fd < NFILES))
		return -EBADF;
	
    file_t* file = NULL;
	if ((file = fget(fd)) == NULL)
	{
		return -EBADF;
	}
    
	vnode_t* vnode = file->f_vnode;
	KASSERT(vnode);
    
	switch(whence)
	{
        case SEEK_SET:
        {
            if (offset < 0)
            {
                fput(file);
                return -EINVAL;
            }
            else
                file->f_pos = offset;
            break;
        }
        case SEEK_CUR:
        {
            if (file->f_pos < -offset)
            {
                fput(file);
                return -EINVAL;
            }
            else
                file->f_pos += offset;
            break;
        }
        case SEEK_END:
        {
            if (offset + vnode->vn_len < 0)
            {
                fput(file);
                return -EINVAL;
            }
            else
                file->f_pos = vnode->vn_len + offset;
            break;
        }
        default:
        {
            fput(file);
            return -EINVAL;
        }
	}
	int off = file->f_pos;
	fput(file);
    
	return off;
}

/*
 * do_stat:
 * Find the vnode associated with the path, and call the stat() vnode operation.
 * param *path: the path name string
 * param *buf: the stat object that you want to read into
 * return: 0 on success, or:
 * -ENOENT:A component of path does not exist.
 * -ENOTDIR:A component of the path prefix of path is not a directory.
 * -ENAMETOOLONG:A component of path was too long.
 */
int
do_stat(const char *path, struct stat *buf)
{
	/* empty one*/
	if (path == NULL || strlen(path) == 0)
		return -ENOENT;
    
	KASSERT(buf != NULL);
	int ret         = 0;
	vnode_t* target = NULL;
    
	if ((ret = open_namev(path, O_RDWR, &target, NULL)) < 0)
	{
		if (target)
			vput(target);
		return ret;
	}
	else
	{
        KASSERT(target);
        vnode_ops_t* ops = target->vn_ops;
        KASSERT(ops);
        KASSERT(ops->stat != NULL);
        ret = ops->stat(target, buf);
        vput(target);
        return ret;
    }   
}
