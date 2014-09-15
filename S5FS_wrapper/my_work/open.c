/*
 *  FILE: open.c
 *  AUTH: Yan Li
 *  DESC: implementation of do_open
 *  TIME: 03/26/2014
 */

#include "globals.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/stat.h"
#include "util/debug.h"

/*
 * get_empty_fd:
 * find empty index in p->p_files[]
 * param *p: the pointer to the process object
 * return: the empty file descriptor, or -EMFILE if we don't have enough file
 *         descriptors
 */
int
get_empty_fd(proc_t *p)
{
    int fd;
    
    for (fd = 0; fd < NFILES; fd++)
    {
        if (!p->p_files[fd])
            return fd;
    }
    
    dbg(DBG_ERROR | DBG_VFS, "ERROR: get_empty_fd: out of file descriptors "
        "for pid %d\n", curproc->p_pid);
    return -EMFILE;
}

/*
 * do_open:
 * open a file given by the file name and oflags
 * param *filename: the name string
 * param oflags: how do you want to open this file? (read, write, read&write, etc)
 *
 * return: the new file descriptor or:
    -EINVAL:oflags is not valid.
 *  -EMFILE:The process already has the maximum number of files open.
 *  -ENOMEM:Insufficient kernel memory was available.
 *  -ENAMETOOLONG:A component of filename was too long.
 *  -ENOENT:O_CREAT is not set and the named file does not exist.
 *  -EISDIR:pathname refers to a directory and the access requested involved
 *         writing
 */

int
do_open(const char *filename, int oflags)
{
    if (!O_ISVALID(oflags))
        return -EINVAL;
    
    int is_rw = oflags & O_RDWR, is_w = oflags & O_WRONLY;
    
    if (is_rw && is_w)
        return -EINVAL;
    int is_r = (!is_rw && !is_w) ? 1 : 0;

    /* Bad truncate */
    if ((oflags & O_TRUNC) && is_r)
    {
        return -EINVAL;
    }
    /* Get the next empty file descriptor */
    int fd = 0;
    if ((fd = get_empty_fd(curproc)) < 0)
        return fd;
    /* Call fget to get a fresh file_t */
    file_t* file = NULL;
    if ((file = fget(-1)) == NULL)
        return -ENOMEM;
  
    /* Save the file_t in curproc's file descriptor table */
    curproc->p_files[fd] = file;
    KASSERT(!curproc->p_files[fd]->f_pos);

    int mode = 0;
    
    if (is_w)
        mode = FMODE_WRITE;
    else if (is_rw)
        mode = FMODE_WRITE | FMODE_READ;
    else
        mode = FMODE_READ;
    
    if ((oflags & O_APPEND ))
        mode = mode | FMODE_APPEND;

    file -> f_mode = mode;
    
    /* Use open_namev() to get the vnode for the file_t.*/
    vnode_t* target = NULL;
    int ret = 0;
    if ((ret = open_namev(filename, oflags, &target, NULL)) < 0)
    {
        /* Implicit errrors: ENOMEM, ENAMETOOLONG*/
        curproc->p_files[fd] = NULL;
        if (target)
            vput(target);
        fput(file);
        return ret;
    }
    KASSERT(target);

    if ((is_w || is_rw) && S_ISDIR(target->vn_mode))
    {
        curproc->p_files[fd] = NULL;
        vput(target);
        fput(file);
        return -EISDIR;
    }
    /*Fill in the fields of the file_t.*/
    curproc->p_files[fd]->f_vnode = target;

    return fd;
}
