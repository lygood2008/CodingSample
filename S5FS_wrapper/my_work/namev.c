/*
 *  FILE: namev.c
 *  AUTH: Yan Li
 *  DESC: subroutines for name opeartions (lookup, find, dir_namv, open_namev)
 *  TIME: 03/26/2014
 */

#include "kernel.h"
#include "globals.h"
#include "types.h"
#include "errno.h"

#include "util/string.h"
#include "util/printf.h"
#include "util/debug.h"

#include "fs/dirent.h"
#include "fs/fcntl.h"
#include "fs/stat.h"
#include "fs/vfs.h"
#include "fs/vnode.h"

/* 
 * lookup:
 * This takes a base 'dir', a 'name', its 'len', and a result vnode.
 * param *dir: vnode object for parent directory
 * param *name: name string
 * param len: the length of name string
 * param **result: *result points to the result vnode object with refcount 
 *                 incremented
 * return: 0 on success;
           -ENOENT: there is no such entry
           -ENOTDIR: it's not a directory
 *         
 */
int
lookup(vnode_t *dir, const char *name, size_t len, vnode_t **result)
{
	if (dir == NULL || name == NULL)
        return -ENOENT;
    
    vnode_ops_t* ops = dir->vn_ops;
    
    KASSERT(ops);
    if (ops->lookup == NULL)
        return -ENOTDIR;
    else
    {
        /* Do not process the .. because it can be found in lookup */
        int ret = 0;
        vnode_t* node;
        if ((ret = ops->lookup(dir, name, len, result)) < 0)
        {
            /* lookup doesn't need to vref, it's already done */
            return ret;
        }
    }
    return 0;
}


/* When successful this function returns data in the following
 *  param *res_vnode: the vnode of the parent directory of "name"
 *  param *name: the `basename'
 *  param namelen: the length of the basename
 *  param *base: the vnode of the base 
 *  param **res_vnode:a successful call to this causes vnode refcount
                      on *res_vnode to
 *  return: 0 on success, or:
            -ENOENT: there is no such entry
            -ENAMETOOLONG: the name is too long
            -NOTDIR: pathname has invalid component (not valid directory)
 */
int
dir_namev(const char *pathname, size_t *namelen, char **name,
          vnode_t *base, vnode_t **res_vnode)
{
	if (pathname == NULL)
		return -ENOENT;
    
	int path_len = strlen(pathname);
	if (path_len >= MAXPATHLEN)
		return -ENAMETOOLONG;
    
	*res_vnode      = NULL;
	vnode_t* parent = NULL;
	
	if (pathname[0] == '/')
	{
		parent = vfs_root_vn;
	}
	else if (base)
	{
		parent = base;
	}
	else
	{
		parent = curproc->p_cwd;
	}

	vref(parent);
    
	int i = 0;
	while (i < path_len)
	{
		if (pathname[i] == '/')
		{
			i++;
			continue;
		}
        else
		{
			int start_index = i;
			int end_index = i + 1;
			int should_terminate = 0;
			while (end_index < path_len && pathname[end_index] != '/')
			{
				end_index++;
			}
			i = end_index;
            
			if (end_index - start_index > NAME_LEN)
			{
				vput(parent);
				return -ENAMETOOLONG;
			}
			if (end_index == path_len)
				should_terminate = 1;
			else
			{
				
				int slash = end_index;
				while (pathname[slash] == '/')
					slash++;
                
				if (slash == path_len)
					should_terminate = 1;
			}

			char tmp_name[NAME_LEN + 1] = {0};
			char* tmp_name_ptr = tmp_name;
			memset(tmp_name, 0, sizeof(tmp_name));
			size_t tmp_name_len = end_index - start_index;
			strncpy(tmp_name, pathname + start_index, tmp_name_len);
			tmp_name[tmp_name_len] = '\0';
            
			if (should_terminate == 1)
			{
				if (!S_ISDIR(parent->vn_mode))
				{
					vput(parent);
					return -ENOTDIR;
				}
				*namelen = tmp_name_len;
				strncpy(*name, tmp_name_ptr, *namelen);
				size_t off = *namelen;
				char* str = *name;
				*(str+off) = '\0';
				*res_vnode = parent;
				return 0;
			}
			int ret = 0;
			vnode_t* next = NULL;
			if ((ret = lookup(parent, tmp_name, tmp_name_len, &next)) < 0)
			{
				vput(parent);
				return ret;
			}
            else
			{
				vput(parent);
				parent = next;
			}
		}
	}
    
	*res_vnode = parent;
	char tmp_name[NAME_LEN + 1];
	tmp_name[0] = '\0';
	memset(tmp_name, 0, sizeof(tmp_name));
	strcpy(*name, tmp_name);
	*namelen = 0;
    
	return 0;
}

/* 
 * open_namev:
 * This returns in res_vnode the vnode requested by the other parameters.
 * It makes use of dir_namev and lookup to find the specified vnode.
 * param *pathname: name string of path name
 * param flag: the flag for creating options
 * param **res_vnode: the refcount on *res_vnode should be incremented after 
                      return
 * param *base: the pointer to the base vnode object
 * return: 0 on success and -errno on failure
 */
int
open_namev(const char *pathname, int flag, vnode_t **res_vnode, vnode_t *base)
{
	vnode_t* dir            = NULL;
    size_t name_len         = 0;
    char name[NAME_LEN + 1] = {0};
    memset(name, 0, sizeof(name));
    char* nameptr = name;
    int ret = 0;
    /* find the parent*/
    if ((ret = dir_namev(pathname, &name_len, &nameptr, base, &dir)) < 0)
    {
        if (dir)
        {
            vput(dir);
        }
        return ret;
    }
    KASSERT(dir);
    vnode_t* target = NULL;
    int ret2 = 0;
    /* See if it's there */
    if ((ret2 = lookup(dir, nameptr, name_len, &target)) == 0)
    {   
        vput(dir);
        *res_vnode = target;
        /* Found it!*/
        return ret2;
    }
    else if (ret2 < 0)
    {
        if (ret2 == -ENOENT)
        {
            if (flag & O_CREAT)
            {
                vnode_ops_t* ops = dir->vn_ops;
                KASSERT(ops);
                KASSERT(ops->create);
               
                int ret3 = 0;
                if ((ret3 = ops->create(dir, nameptr, name_len, &target)) < 0)
                {
                    if (target)
                    {
                        vput(target);
                    }
                }
                *res_vnode = target;
                vput(dir);
                return ret3;
            }
        }
        /* error in lookup */
        vput(dir);
        return ret2;
    }
    KASSERT(0);
    return 0;
}
