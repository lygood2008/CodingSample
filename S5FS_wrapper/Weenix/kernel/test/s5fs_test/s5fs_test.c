/*
 *  FILE: s5fs_test.c
 *  AUTHOR: Yan Li
 *  DESCR: Unit test for S5FS
 *  TIME: 05/04/2014
 */
#pragma once

#include "kernel.h"
#include "globals.h"
#include "errno.h"
#include "config.h"
#include "limits.h"

#include "util/debug.h"
#include "util/string.h"
#include "util/printf.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "fs/dirent.h"
#include "fs/vfs_syscall.h"
#include "fs/stat.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/mman.h"
#include "mm/kmalloc.h"
#include "test/usertest.h"

/* errno */
#define errno (curthr->kt_errno)

/* general buffer size */
#define BUF_SIZE 256

/*
 * Hooks
 */
#define mkdir(a, b)       do_mkdir_wrap(a)
#define mknod(a, b, c)    do_mknod_wrap(a, b, c)
#define rmdir(a)          do_rmdir_wrap(a)
#define mount(a)          do_mount_wrap(a)
#define umount(a)         do_umount_wrap(a)
#define open(a, b, c)     do_open_wrap(a, b)
#define close(a)          do_close_wrap(a)
#define link(a, b)        do_link_wrap(a, b)
#define rename(a, b)      do_rename_wrap(a, b)
#define unlink(a)         do_unlink_wrap(a)
#define read(a, b, c)     do_read_wrap(a, b, c)
#define write(a, b, c)    do_write_wrap(a, b, c)
#define lseek(a, b, c)    do_lseek_wrap(a, b, c)
#define dup(a)            do_dup_wrap(a)
#define dup2(a, b)        do_dup2_wrap(a, b)
#define chdir(a)          do_chdir_wrap(a)
#define stat(a, b)        do_stat_wrap(a, b)
#define getdents(a, b, c) do_getdents(a,b,c)
#define exit(a)           do_exit(a)
/* Some helpful strings */
#define LONG_STR "adsasdasdlwlelqwelqwdsfsdmfmsdfwww" /* Longer than NAME_LEN */
#define SHORT_STR "abcdefgh ijklmn"

static char root[BUF_SIZE] = {0};

/* Forward declarations */

static int syscall_success(int expected);
static int syscall_fail(int expected, int err);

/* Environments */
static void s5fstest_start(void);
static void s5fstest_term(void);

static int do_close_wrap(int fd);
static int do_read_wrap(int fd, void *buf, size_t nbytes);
static int do_write_wrap(int fd, const void *buf, size_t nbytes);
static int do_dup_wrap(int fd);
static int do_dup2_wrap(int ofd, int nfd);
static int do_mkdir_wrap(const char *path);
static int do_mknod_wrap(const char *path, int mode, int devid);
static int do_rmdir_wrap(const char *path);
static int do_link_wrap(const char *old, const char *new);
static int do_rename_wrap(const char *oldname, const char *newname);
static int do_unlink_wrap(const char *path);
static int do_chdir_wrap(const char *path);
static int do_lseek_wrap(int fd, int offset, int whence);
static int do_stat_wrap(const char *path, struct stat *uf);
static int do_open_wrap(const char *filename, int flags);

/* Utilities */
static int do_getdents(int fd, struct dirent *dirp, unsigned int count);
static int getdent(const char *dir, dirent_t *dirent)
;
static int make_dirs(const char *dir);
static int remove_recur(const char *dir);

/* Tests */
static void s5fstest_stat(void);
static void s5fstest_chdir(void);
static void s5fstest_mkdir_rmdir(void);
static void s5fstest_mknod(void);
static void s5fstest_paths(void);
static void s5fstest_close(void);
static void s5fstest_unlink(void);
static void s5fstest_fd(void);
static void s5fstest_open_close_close(void);
static void s5fstest_read(void);
static void s5fstest_getdents(void);

#define syscall_fail(expr, err)                                                \
(test_assert((errno = 0, -1 == (expr)),                                    \
"\nunexpected success, wanted %s (%d)",                                   \
test_errstr(err), err) ?                                                   \
test_assert((expr, errno == err), "\nexpected %s (%d)"                     \
"\ngot      %s (%d)",                                                      \
test_errstr(err), err,                                                     \
test_errstr(errno), errno) : 0)

#define syscall_success(expr)                                                  \
test_assert(0 <= (expr), "\nunexpected error: %s (%d)",                    \
test_errstr(errno), errno)


/*
 * starts the testing environment
 */
static void s5fstest_start(void)
{
    dbg(DBG_TEST, "Now creating test root directory: %s\n", root);
    sprintf(root, "s5fs_test");
    int err;
    if ((err = mkdir(root, 0777)) < 0)
    {
        dbg(DBG_TEST, "Error.");
        exit(-1);
    }
    dbg(DBG_TEST, "Success.");
}

/*
 * Terminates the testing environment
 */
static void s5fstest_term(void)
{
    dbg(DBG_TEST, "Now deleting test root directory: %s\n", root);
    if (0 != remove_recur(root))
    {
        dbg(DBG_TEST,"ERROR.");
        exit(-1);
    }
    dbg(DBG_TEST, "Success");
}

/* Wrappers */

static int do_close_wrap(int fd)
{
    int ret = do_close(fd);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_read_wrap(int fd, void *buf, size_t nbytes)
{
    int ret = do_read(fd, buf, nbytes);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_write_wrap(int fd, const void *buf, size_t nbytes)
{
    int ret = do_write(fd, buf, nbytes);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_dup_wrap(int fd)
{
    int ret = do_dup(fd);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_dup2_wrap(int ofd, int nfd)
{
    int ret = do_dup2(ofd, nfd);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_mkdir_wrap(const char *path)
{
    int ret = do_mkdir(path);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_mknod_wrap(const char *path, int mode, int devid)
{
    int ret = do_mknod(path, mode, devid);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_rmdir_wrap(const char *path)
{
    int ret = do_rmdir(path);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_link_wrap(const char *old, const char *new)
{
    int ret = do_link(old, new);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_rename_wrap(const char *old, const char *new)
{
    int ret = do_rename(old, new);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_unlink_wrap(const char *path)
{
    int ret = do_unlink(path);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_chdir_wrap(const char *path)
{
    int ret = do_chdir(path);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_lseek_wrap(int fd, int offset, int whence)
{
    int ret = do_lseek(fd, offset, whence);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_stat_wrap(const char *path, struct stat *uf)
{
    int ret = do_stat(path, uf);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

static int do_open_wrap(const char *filename, int flags)
{
    int ret = do_open(filename, flags);
    if(ret < 0){errno = -ret;return -1;}
    return ret;
}

/*
 * read_helper:
 * a wrapper to test read
 * param fd: file descriptor
 * param size: expected size
 * param *goal: buffer
 */
#define read_helper(fd, size, goal)\
do\
{\
char buf[BUF_SIZE] = {0};\
test_assert((ssize_t)strlen(goal) == read(fd, buf, size),\
"\nread incorrect bytes");\
test_assert(!memcmp(buf, goal, strlen(goal)), "\ndata is wrong");\
} while(0);

/*
 * create_file_helper:
 * a wrapper to test open file in create mode
 * param *file file name
 */
#define create_file_helper(file)\
do\
{\
int fd;\
if (syscall_success(fd = open(file, O_RDONLY | O_CREAT, 0777)))\
{\
syscall_success(close(fd));\
}\
} while(0);



/* file_pos_helper:
 * test the file position
 * param fd: the file descriptor
 * param expected: expected pos
 */
#define file_pos_helper(fd, expected)\
do\
{\
int g;\
int e = expected;\
syscall_success(g = lseek(fd, 0, SEEK_CUR));\
test_assert((g == e), "Wrong fpos at %d, expected %d", fd, g, e);\
} while(0);

/*
 * paths_qual_helper:
 * compare the path
 * param path1: path1
 * param path2: path2
 */
#define paths_qual_helper(path1, path2)\
do\
{\
int ret = 0;\
struct stat stat1;\
struct stat stat2;\
ret = make_dirs(path1);\
test_assert(!ret, "make a temp dir:\"%s\": (error): %s", path1,\
test_errstr(ret));\
\
ret = stat(path1, &stat1);\
test_assert(!ret, "stat1:\"%s\": (error): %s", path1,\
test_errstr(errno));\
\
ret = stat(path2, &stat2);\
test_assert(!ret, "stat2:\"%s\": (error): %s", path2,\
test_errstr(errno));\
test_assert(stat1.st_ino == stat2.st_ino,\
"paths_equals(\"%s\" (inode %d), \"%s\" (inode %d))",\
path1, stat1.st_ino, path2, stat2.st_ino);\
} while(0);
/*
 * do_getdents:
 * get all of the dirent from the directory given by the file descriptor
 * param fd: the file descriptor
 * param *dirp: the dirent array
 * param count: the size of the array
 * return: -1 on failure, and postive number for number of bytes that have been
 *         read
 */
static int do_getdents(int fd, struct dirent *dirp, unsigned int count)
{
    int bytes             = 0;
    size_t num_bytes_read = 0;
    
    dirent_t temp_dirent;
    
    while (num_bytes_read < count)
    {
        bytes = do_getdent(fd, &temp_dirent);
        if (bytes < 0)
        {
            errno = -bytes;
            return -1;
        }
        else if (bytes == 0)
        {
            return num_bytes_read;
        }
        
        memcpy(dirp, &temp_dirent, sizeof(dirent_t));
        dirp++;
        num_bytes_read += bytes;
    }
    return num_bytes_read;
}

/* getdent:
 * get dirents within dir
 * param dir: the directory name
 * param dirent: the dirent array
 * return: 0 on success, or negative number for failure
 */
static int getdent(const char *dir, dirent_t *dirent)
{
    dbg(DBG_TEST, "{\n");
    int ret = 0;
    int fd = -1;
    
    if ((fd = open(dir, O_RDONLY, 777)) < 0)
    {
        dbg(DBG_TEST, "error: %d\n", fd);
        return -1;
    }
    
    ret = 1;
    while (ret != 0)
    {
        if ((ret = getdents(fd, dirent, sizeof(*dirent))) < 0)
        {
            dbg(DBG_TEST,"error: %d\n", ret);
            return -1;
        }
        if (strcmp(".", dirent->d_name) && strcmp("..", dirent->d_name))
        {
            close(fd);
            dbg(DBG_TEST, "}\n");
            return 1;
        }
    }
    
    close(fd);
    dbg(DBG_TEST, "}\n");
    return 0;
}

/* make_dirs:
 * make a directory with the name specified
 * param dir: the directory name
 * return: 0 on success, or negative number for failure
 */
static int make_dirs(const char *dir)
{
    dbg(DBG_TEST, "{\n");
    char *d = kmalloc(strlen(dir) + 1);
    
    if(d == NULL)
    {
        return ENOMEM;
    }
    char *p = NULL;
    
    strcpy(d, dir);
    
    p = d;
    
    while ((p = strchr(p + 1, '/')))
    {
        *p = '\0';
        if (mkdir(d, 0777) &&
            EEXIST != errno)
        {
            dbg(DBG_TEST,"error: %d\n", errno);
            return errno;
        }
        *p = '/';
    }
    if (mkdir(d, 0777) &&
        EEXIST != errno)
    {
        dbg(DBG_TEST,"error, %d\n", errno);
        return errno;
    }
    
    dbg(DBG_TEST, "}\n");
    return 0;
}

/* remove_recur:
 * recursively remove the directory
 * param dir: the directory name
 * return: 0 on success, or negative number for failure
 */
static int remove_recur(const char *dir)
{
    dbg(DBG_TEST, "{\n");
    int ret = 0;
    int fd  = -1;
    int err = 0;
    dirent_t dirent;
    struct stat status;
    
    if (chdir(dir) < 0)
        return errno;
    
    ret = 1;
    while (ret)
    {
        ret = getdent(".", &dirent);
        if (ret < 0)
        {
            err = 1;
            break;
        }
        else if (!ret)
            break;
        
        if (stat(dirent.d_name, &status) < 0)
        {
            err = 1;
            break;
        }
        
        if (S_ISDIR(status.st_mode))
        {
            if (remove_recur(dirent.d_name) < 0)
            {
                err = 1;
                break;
            }
        }
        else
        {
            if(unlink(dirent.d_name) < 0)
            {
                err = 1;
                break;
            }
        }
    }
    
    /* Go back */
    if (chdir("..") < 0)
        err = 1;
    /* remove the original one */
    if (rmdir(dir) < 0)
        err = 1;
    
    close(fd);
    if (err)
    {
        dbg(DBG_TEST, "error.%d\n", errno);
        return errno;
    }
    dbg(DBG_TEST, "}\n");
    return 0;
}


/*
 * s5fstest_stat:
 * Test stat
 */
static void s5fstest_stat(void)
{
    int fd;
    struct stat s;
    const char* stat_test_dir = "stat";
    syscall_success(mkdir(stat_test_dir, 0));
    syscall_success(chdir(stat_test_dir));
    
    syscall_success(stat(".", &s));
    test_assert(S_ISDIR(s.st_mode), NULL);
    
    create_file_helper("f");
    syscall_success(stat("f", &s));
    test_assert(S_ISREG(s.st_mode), NULL);
    syscall_fail(stat("what", &s), ENOENT);
    syscall_fail(stat(NULL, &s), ENOENT);
    /* file size is correct */
    syscall_success(fd = do_open("f", O_RDWR));
    syscall_success(write(fd, "test", 4));
    syscall_success(stat("f", &s));
    test_assert(s.st_size == 4, "unexpected file size");
    test_assert(s.st_blksize == 4096, "unexpected page size");
    test_assert(s.st_nlink == 1, "unexpected link number");
    syscall_success(write(fd, LONG_STR, strlen(LONG_STR)));
    syscall_success(stat("f", &s));
    test_assert(s.st_size == (ssize_t)(4 + strlen(LONG_STR)), "unexpected file size");
    syscall_success(close(fd));
    
    /* open again */
    syscall_success(fd = do_open("f", O_RDWR));
    syscall_success(stat("f", &s));
    test_assert(s.st_size == (ssize_t)(4 + strlen(LONG_STR)), "unexpected file size");
    test_assert(s.st_blksize == 4096, "unexpected page size");
    test_assert(s.st_nlink == 1, "unexpected link number");
    syscall_success(close(fd));
    syscall_success(chdir(".."));
    syscall_success(remove_recur(stat_test_dir));
}

/*
 * s5fstest_chdir:
 * Test chdir
 */
static void s5fstest_chdir(void)
{
    const char* chdir_test_dir = "chdir";
    
    struct stat sdot, sdotdot, sdir, ddot, ddotdot;
    
    /* change directory back and forth*/
    syscall_success(mkdir(chdir_test_dir, 0777));
    syscall_success(stat(".", &sdot));
    syscall_success(stat("..", &sdotdot));
    syscall_success(stat(chdir_test_dir, &sdir));
    syscall_success(chdir(chdir_test_dir));
    
    syscall_success(stat(".", &ddot));
    syscall_success(stat("..", &ddotdot));
    
    test_assert(sdot.st_ino != ddot.st_ino, NULL);
    test_assert(sdotdot.st_ino != ddotdot.st_ino, NULL);
    test_assert(sdot.st_ino == ddotdot.st_ino, NULL);
    test_assert(sdir.st_ino == ddot.st_ino, NULL);
    test_assert(sdir.st_ino != ddotdot.st_ino, NULL);
    
    /* chage to itself */
    syscall_success(chdir("."));
    syscall_success(stat(".", &ddot));
    syscall_success(stat("..", &ddotdot));
    /* should not change anything */
    test_assert(sdot.st_ino != ddot.st_ino, NULL);
    test_assert(sdotdot.st_ino != ddotdot.st_ino, NULL);
    test_assert(sdot.st_ino == ddotdot.st_ino, NULL);
    test_assert(sdir.st_ino == ddot.st_ino, NULL);
    test_assert(sdir.st_ino != ddotdot.st_ino, NULL);
    
    /* failure test */
    create_file_helper("tmp");
    syscall_fail(chdir("tmp"), ENOTDIR);
    syscall_fail(chdir("loveyou"), ENOENT);
    syscall_fail(chdir(NULL), ENOENT);
    
    /* go back */
    syscall_success(chdir(".."));
    syscall_success(stat(".", &sdot));
    syscall_success(stat("..", &sdotdot));
    
    /* should not change anything */
    test_assert(sdot.st_ino != ddot.st_ino, NULL);
    test_assert(sdotdot.st_ino != ddotdot.st_ino, NULL);
    test_assert(sdot.st_ino == ddotdot.st_ino, NULL);
    test_assert(sdir.st_ino == ddot.st_ino, NULL);
    test_assert(sdir.st_ino != ddotdot.st_ino, NULL);
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(chdir_test_dir));
}

/*
 * s5fstest_mkdir_rmdir:
 * Test mkdir, rmdir
 */
static void s5fstest_mkdir_rmdir(void)
{
    const char* mkdir_test_dir = "mkdir";
    syscall_success(mkdir(mkdir_test_dir, 0777));
    syscall_success(chdir(mkdir_test_dir));
    
    syscall_success(mkdir("dir", 0777));
    syscall_success(mkdir(SHORT_STR, 0777));
    syscall_success(mkdir(mkdir_test_dir, 0777));
    
    /* failure test*/
    syscall_fail(mkdir(NULL, 0777), ENOENT);
    create_file_helper("tmp");
    syscall_fail(mkdir("tmp", 0777), EEXIST);
    syscall_fail(mkdir("dir", 0777), EEXIST);
    syscall_fail(mkdir("/", 0777), EEXIST);
    syscall_fail(mkdir(".", 0777), EEXIST);
    syscall_fail(mkdir("..", 0777), EEXIST);
    syscall_fail(mkdir(LONG_STR, 0777), ENAMETOOLONG);
    syscall_fail(mkdir("tmp/dir", 0777), ENOTDIR);
    syscall_fail(mkdir("love/dir", 0777), ENOENT);
    syscall_fail(rmdir("tmp/dir"), ENOTDIR);
    syscall_fail(rmdir("love/dir"), ENOENT);
    syscall_fail(rmdir("love"), ENOENT);
    syscall_fail(rmdir("/"), ENOTEMPTY);
    syscall_fail(rmdir("."), EINVAL);
    syscall_fail(rmdir(".."), ENOTEMPTY);
    syscall_fail(rmdir("dir/."), EINVAL);
    syscall_fail(rmdir("dir/.."), ENOTEMPTY);
    syscall_fail(rmdir("love/."), ENOENT);
    syscall_fail(rmdir("love/.."), ENOENT);
    create_file_helper("dir/tmp");
    syscall_fail(rmdir("dir"), ENOTEMPTY);
    syscall_success(unlink("dir/tmp"));
    syscall_success(rmdir("dir"));
    syscall_success(mkdir("dir", 0777));
    syscall_success(chdir(".."));
    
    syscall_success(remove_recur(mkdir_test_dir));
}

/*
 * s5fstest_mknod:
 * Test mknod
 */
static void s5fstest_mknod(void)
{
    const char* mkdir_test_dir = "mkdir";
    syscall_success(mkdir(mkdir_test_dir, 0777));
    syscall_success(chdir(mkdir_test_dir));
    
    syscall_success(mknod("m1", S_IFCHR, 1));
    syscall_success(mknod("m2", S_IFBLK, 2));
    
    /* Invalid name */
    syscall_fail(mknod("m3", S_IFBLK | S_IFCHR, 1), EINVAL);
    syscall_fail(mknod("m1", S_IFCHR, 1), EEXIST);
    syscall_fail(mknod("/", S_IFCHR, 1), EEXIST);
    syscall_fail(mknod(".", S_IFCHR, 1), EEXIST);
    syscall_fail(mknod("..", S_IFCHR, 1), EEXIST);
    
    syscall_fail(mknod("m3",S_IFDIR, 2), EINVAL);
    syscall_fail(mknod("m3",S_IFREG, 2), EINVAL);
    syscall_fail(mknod("m3", S_IFLNK, 2), EINVAL);
    
    syscall_success(chdir(".."));
    
    syscall_success(remove_recur(mkdir_test_dir));
}

/*
 * s5fstest_path:
 * Test path
 */
static void s5fstest_paths(void)
{
    const char* path_test_dir = "paths";
    struct stat s;
    syscall_success(mkdir(path_test_dir, 0777));
    syscall_success(chdir(path_test_dir));
    
    paths_qual_helper(".", ".");
    paths_qual_helper("..", "..");
    paths_qual_helper("1", "1");
    paths_qual_helper("1/2", "1/2");
    paths_qual_helper("1/2/3", "1/2/3");
    paths_qual_helper("4/5/6", "4/5/6");
    
    /* root */
    paths_qual_helper("/", "/");
    paths_qual_helper("/", "/..");
    paths_qual_helper("/", "/../");
    paths_qual_helper("/", "/./");
    paths_qual_helper("/", "/./..");
    paths_qual_helper("/", "/./../");
    paths_qual_helper("/", "/../.");
    paths_qual_helper("/", "/.././");
    paths_qual_helper("/", "/./.");
    paths_qual_helper("/", "/././");
    paths_qual_helper("/", "/../..");
    paths_qual_helper("/", "/../../");
    
    
    /* . and .. */
    paths_qual_helper("../.", "./..");
    paths_qual_helper(".", "./.");
    paths_qual_helper(".", "1/..");
    paths_qual_helper(".", "1/../");
    paths_qual_helper(".", "1/2/../..");
    paths_qual_helper(".", "1/2/../..");
    paths_qual_helper(".", "1/2/3/../../..");
    paths_qual_helper(".", "1/../1/..");
    paths_qual_helper(".", "1/../4/..");
    paths_qual_helper(".", "1/2/../../4/5/../..");
    paths_qual_helper(".", "1/2/3/../../../4/5/6/../../..");
    paths_qual_helper(".", "1/./../1/2/./../../1/2/3/"
                      "./../../../4/./../4/5/./../../4/5/6/../../..");
    
    /* extra slashes */
    paths_qual_helper("/", "//");
    paths_qual_helper("/", "///////");
    paths_qual_helper("1/2/3", "1/2/3/");
    paths_qual_helper("1/2/3", ".//1/2/3/");
    paths_qual_helper("1/2/3", "1//2/3");
    paths_qual_helper("1/2/3", "1/2//3");
    paths_qual_helper("1/2/3", "1/2/3//");
    paths_qual_helper("1/2/3", ".//1//2//3");
    paths_qual_helper("1/2/3", ".//1//2//3/");
    paths_qual_helper("1/2/3", ".///1///2///3///");
    
    /* strange names */
    paths_qual_helper("1", "1");
    paths_qual_helper("-", "-");
    paths_qual_helper("-1", "-1");
    paths_qual_helper(" ", " ");
    paths_qual_helper("    ", "    ");
    paths_qual_helper("\\", "\\");
    paths_qual_helper("%%", "%%");
    paths_qual_helper("*", "*");
    paths_qual_helper(">", ">");
    
    struct stat st;
    
    /* errors */
    syscall_fail(stat("pwd", &st), ENOENT);
    syscall_fail(stat("1/pwd", &st), ENOENT);
    syscall_fail(stat("1/./pwd", &st), ENOENT);
    syscall_fail(stat("1/../pwd", &st), ENOENT);
    syscall_fail(stat("1/2/pwd", &st), ENOENT);
    syscall_fail(stat("1/2/3/pwd", &st), ENOENT);
    
    create_file_helper("tmp");
    syscall_fail(open("tmp/1", O_RDONLY, 0777), ENOTDIR);
    syscall_fail(open("tmp/1/..", O_RDONLY | O_CREAT, 0777), ENOTDIR);
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(path_test_dir));
}

/*
 * s5fstest_close:
 * Test close
 */
static void s5fstest_close(void)
{
    /* Basically just open and close twice */
    const char* close_test_dir = "close";
    syscall_success(mkdir(close_test_dir, 0777));
    syscall_success(chdir(close_test_dir));
    
    int fd;
    const int bad_fd = 20;
    const int huge_fd = 9999;
    syscall_success(fd = open("tmp", O_WRONLY | O_CREAT, 0));
    syscall_success(close(fd));
    /* close twice */
    syscall_fail(close(fd), EBADF);
    /* close arbitratry*/
    syscall_fail(close(huge_fd), EBADF);
    syscall_fail(close(bad_fd), EBADF);
    
    syscall_success(fd = open("tmp", O_WRONLY | O_CREAT, 0));
    /* do close after unlink */
    syscall_success(unlink("tmp"));
    syscall_success(close(fd));
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(close_test_dir));
}

/*
 * s5fstest_unlink:
 * Test unlink
 */
static void s5fstest_unlink(void)
{
    const char* unlink_test_dir = "close";
    syscall_success(mkdir(unlink_test_dir, 0777));
    syscall_success(chdir(unlink_test_dir));
    int fd;
    syscall_success(fd = open("tmp1", O_WRONLY | O_CREAT, 0));
    syscall_success(close(fd));
    syscall_success(unlink("tmp1"));
    
    syscall_success(mkdir("dir", 0777));
    /* unlink a directory */
    syscall_fail(unlink("dir"), EISDIR);
    
    /* unlink a non existent path*/
    syscall_fail(unlink("nonexistent"), ENOENT);
    
    /* unlink twice */
    syscall_success(fd = open("tmp2", O_WRONLY | O_CREAT, 0));
    syscall_success(close(fd));
    syscall_success(unlink("tmp2"));
    syscall_fail(unlink("tmp2"), ENOENT);
    
    /* use dup and unlink twice */
    syscall_success(fd = open("tmp3", O_WRONLY | O_CREAT, 0));
    int fd2;
    syscall_success(fd2 = dup(fd));
    syscall_success(unlink("tmp3"));
    syscall_fail(unlink("tmp3"), ENOENT);
    syscall_success(close(fd));
    syscall_success(close(fd2));
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(unlink_test_dir));
}

/*
 * s5fstest_fd:
 * Test file descriptor
 */
static void s5fstest_fd(void)
{
    const int fd_buf_size = 5;
    const int bad_fd = 20;
    const int huge_fd = 9999;
    
    int fd1, fd2;
    char buf[BUF_SIZE];
    struct dirent d;
    
    const char* fd_test_dir = "fd";
    
    syscall_success(mkdir(fd_test_dir, 0));
    syscall_success(chdir(fd_test_dir));
    
    /* read/write/close/getdents/dup nonexistent file descriptors */
    /* only those functions have file descriptor as parameter */
    syscall_fail(read(bad_fd, buf, fd_buf_size), EBADF);
    syscall_fail(read(huge_fd, buf, fd_buf_size), EBADF);
    syscall_fail(read(-1, buf, fd_buf_size), EBADF);
    
    syscall_fail(write(bad_fd, buf, fd_buf_size), EBADF);
    syscall_fail(write(huge_fd, buf, fd_buf_size), EBADF);
    syscall_fail(write(-1, buf, fd_buf_size), EBADF);
    
    syscall_fail(close(bad_fd), EBADF);
    syscall_fail(close(huge_fd), EBADF);
    syscall_fail(close(-1), EBADF);
    
    syscall_fail(lseek(bad_fd, 0, SEEK_SET), EBADF);
    syscall_fail(lseek(huge_fd, 0, SEEK_SET), EBADF);
    syscall_fail(lseek(-1, 0, SEEK_SET), EBADF);
    
    syscall_fail(getdents(bad_fd, &d, sizeof(d)), EBADF);
    syscall_fail(getdents(huge_fd, &d, sizeof(d)), EBADF);
    syscall_fail(getdents(-1, &d, sizeof(d)), EBADF);
    
    syscall_fail(dup(bad_fd), EBADF);
    syscall_fail(dup(huge_fd), EBADF);
    syscall_fail(dup(-1), EBADF);
    
    syscall_fail(dup2(bad_fd, 10), EBADF);
    syscall_fail(dup2(huge_fd, 10), EBADF);
    syscall_fail(dup2(-1, 10), EBADF);
    
    syscall_fail(dup2(0, bad_fd), EBADF);
    syscall_fail(dup2(0, huge_fd), EBADF);
    syscall_fail(dup2(0, -1), EBADF);
    
    syscall_fail(dup2(bad_fd, bad_fd), EBADF);
    syscall_fail(dup2(huge_fd, huge_fd), EBADF);
    syscall_fail(dup2(-1, -1), EBADF);
    
    /* dup works properly in normal usage */
    create_file_helper("tmp01");
    
    syscall_success(fd1 = open("tmp01", O_RDWR, 0));
    syscall_success(fd2 = dup(fd1));
    
    test_assert(fd1 < fd2, "dup(%d) returned %d", fd1, fd2);
    syscall_success(write(fd2, "helloworld", 10));
    file_pos_helper(fd1, 10);
    file_pos_helper(fd2, 10);
    syscall_success(lseek(fd2, 3, SEEK_SET));
    file_pos_helper(fd1, 3);
    file_pos_helper(fd2, 3);
    
    read_helper(fd1, 3, "low");
    file_pos_helper(fd1, 6);
    file_pos_helper(fd2, 6);
    syscall_success(close(fd2));
    
    /* dup2 works properly in normal usage */
    syscall_success((fd2 = dup2(fd1, 10)) == 10);
    file_pos_helper(fd1, 6);
    file_pos_helper(fd2, 6);
    syscall_success(lseek(fd2, 0, SEEK_SET));
    file_pos_helper(fd1, 0);
    file_pos_helper(fd2, 0);
    /* overwrite */
    syscall_success(write(fd2, "helloworld", 10));
    file_pos_helper(fd1, 10);
    file_pos_helper(fd2, 10);
    syscall_success(close(fd2));
    
    /* dup2 to itself */
    syscall_success((fd2 = dup2(fd1, fd1)) == fd1);
    syscall_success(close(fd2));
    
    /* concatenate dup, will make fd1 give up its current one  */
    int fd3;
    create_file_helper("tmp02");
    syscall_success(fd3 = open("tmp02", O_RDWR, 0));
    syscall_success((fd2 = dup2(fd1, fd3)) == fd3);
    file_pos_helper(fd1, 0);
    file_pos_helper(fd2, 0);
    syscall_success(write(fd1, "helloworld", 10));
    syscall_success(lseek(fd3, 5, SEEK_SET));
    file_pos_helper(fd1, 5);
    file_pos_helper(fd2, 5);
    read_helper(fd3, 5, "world");
    syscall_success(close(fd2));
    
    syscall_success(chdir(".."));
    
    syscall_success(remove_recur(fd_test_dir));
}

/*
 * s5fstest_open_close:
 * Test open
 */
static void s5fstest_open_close(void)
{
    const char* open_test_dir = "open";
    char buf[BUF_SIZE] = {0};
    int fd, fd2;
    struct stat s;
    
    syscall_success(mkdir(open_test_dir, 0777));
    syscall_success(chdir(open_test_dir));
    
    syscall_fail(open("tmp1", O_WRONLY | O_RDWR | O_CREAT, 0), EINVAL);
    syscall_fail(open("tmp1", O_RDONLY | O_RDWR | O_WRONLY | O_CREAT, 0), EINVAL);
    syscall_fail(open("tmp1", O_RDWR | O_WRONLY | O_CREAT, 0), EINVAL);
    
    /* Cannot open nonexistent file without O_CREAT */
    syscall_fail(open("tmp2", O_WRONLY, 0), ENOENT);
    syscall_success(fd = open("tmp2", O_RDONLY | O_CREAT, 0));
    syscall_success(close(fd));
    syscall_success(unlink("tmp2"));
    syscall_fail(unlink("tmp2"), ENOENT);
    
    /* Cannot create invalid files */
    create_file_helper("tmp3");
    syscall_fail(open("tmp3/tmp", O_RDONLY | O_CREAT, 0), ENOTDIR);
    syscall_fail(open("love/tmp", O_RDONLY | O_CREAT, 0), ENOENT);
    syscall_fail(open(LONG_STR, O_RDONLY | O_CREAT, 0), ENAMETOOLONG);
    syscall_success(fd = open(SHORT_STR, O_RDONLY | O_CREAT, 0));
    syscall_success(close(fd));
    
    /* Append mode */
    syscall_success(fd = open("tmp4", O_WRONLY | O_CREAT, 0));
    syscall_success(write(fd, "test", 4));
    syscall_success(close(fd));
    syscall_success(fd = open("tmp4", O_WRONLY | O_APPEND | O_CREAT, 0));
    syscall_success(write(fd, "love", 4));
    file_pos_helper(fd, 8);
    syscall_success(close(fd));
    
    /* Cannot write to readonly file */
    syscall_success(fd = open("tmp5", O_RDONLY | O_CREAT, 0));
    syscall_fail(write(fd, "hello", 5), EBADF);
    syscall_success(close(fd));
    
    /* open a directory for writing */
    syscall_success(mkdir("dir", 0));
    syscall_fail(open("dir", O_RDWR, 0), EISDIR);
    syscall_success(rmdir("dir"));
    
    /* open a directory for reading */
    syscall_success(mkdir("dir", 0));
    syscall_success(fd = open("dir", O_RDONLY, 0));
    syscall_success(rmdir("dir"));
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(open_test_dir));
}

/*
 * s5fstest_fd:
 * Test read
 */
static void s5fstest_read(void)
{
    int fd, ret;
    char buf[BUF_SIZE];
    struct stat s;
    const char* read_test_dir = "read";
    syscall_success(mkdir(read_test_dir, 0777));
    syscall_success(chdir(read_test_dir));
    
    
    syscall_success(fd = open("tmp1", O_RDWR | O_CREAT, 0));
    syscall_success(ret = write(fd, "hello", 5));
    test_assert(5 == ret, "write(%d, \"hello\", 5) returned %d", fd, ret);
    syscall_success(ret = lseek(fd, 0, SEEK_SET));
    test_assert(!ret, "lseek(%d, 0, SEEK_SET) returned %d", fd, ret);
    read_helper(fd, BUF_SIZE, "hello");
    syscall_success(close(fd));
    
    /* Cannot read from write only file. */
    syscall_success(fd = open("tmp2", O_WRONLY | O_CREAT, 0));
    syscall_fail(read(fd, buf, BUF_SIZE), EBADF);
    syscall_success(close(fd));
    syscall_success(unlink("tmp2"));
    
    /* Cannot read from directory */
    syscall_success(mkdir("dir", 0));
    syscall_success(fd = open("dir", O_RDONLY, 0));
    syscall_fail(read(fd, buf, BUF_SIZE), EISDIR);
    syscall_success(close(fd));
    
    syscall_success(fd = open("tmp3", O_RDWR | O_CREAT, 0));
    syscall_success(write(fd, "hello", 5));
    
    /* Test seek cur */
    syscall_success(ret = lseek(fd, 0, SEEK_CUR));
    test_assert(ret == 5, "lseek(%d, 0, SEEK_CUR) returned %d", fd, ret);
    
    read_helper(fd, 10, "");
    
    syscall_success(ret = lseek(fd, -1, SEEK_CUR));
    test_assert(ret == 4, "lseek(%d, -1, SEEK_CUR) returned %d", fd, ret);
    read_helper(fd, 10, "o");
    
    syscall_success(ret = lseek(fd, 2, SEEK_CUR));
    test_assert(ret == 7, "lseek(%d, 2, SEEK_CUR) returned %d", fd, ret);
    read_helper(fd, 10, "");
    
    syscall_fail(ret = lseek(fd, -8, SEEK_CUR), EINVAL);
    
    /* Test seek set */
    syscall_success((ret = lseek(fd, 0, SEEK_SET)) == 0);
    read_helper(fd, 10, "hello");
    syscall_fail(lseek(fd, -1, SEEK_SET), EINVAL);
    syscall_success(ret = lseek(fd, 20, SEEK_SET));
    test_assert(ret == 20, "lseek(%d, 20, SEEK_SET) returned %d", fd, ret);
    syscall_success(write(fd, "love", 4));
    syscall_success(ret = lseek(fd, 20, SEEK_SET));
    test_assert(ret == 20, "lseek(%d, 20, SEEK_SET) returned %d", fd, ret);
    read_helper(fd, 4, "love");
    
    syscall_success(ret = lseek(fd, 18, SEEK_SET));
    test_assert(ret == 18, "lseek(%d, 18, SEEK_SET) returned %d", fd, ret);
    syscall_success(write(fd, "what", 4));
    syscall_success(ret = lseek(fd, 20, SEEK_SET));
    test_assert(ret == 20, "lseek(%d, 20, SEEK_SET) returned %d", fd, ret);
    read_helper(fd, 4, "atve");
    
    /* Test seek end */
    syscall_success(ret = lseek(fd, 0, SEEK_END));
    test_assert(ret == 24, "lseek(%d, 0, SEEK_END) returned %d", fd, ret);
    read_helper(fd, 10, "");
    
    syscall_success(ret = lseek(fd, -1, SEEK_END));
    test_assert(ret == 23, "lseek(%d, 0, SEEK_END) returned %d", fd, ret);
    read_helper(fd, 10, "e");
    
    syscall_success(ret = lseek(fd, -3, SEEK_END));
    test_assert(ret == 21, "lseek(%d, -3, SEEK_END) returned %d", fd, ret);
    read_helper(fd, 10, "tve");
    
    syscall_success(ret = lseek(fd, 3, SEEK_END));
    test_assert(ret == 27, "lseek(%d, 3, SEEK_END) returned %d", fd, ret);
    read_helper(fd, 10, "");
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(read_test_dir));
}

/*
 * s5fstest_getdents:
 * Test getdents
 */
static void s5fstest_getdents(void)
{
    int fd  = 0;
    int ret = 0;
    dirent_t dirents[5];
    
    const char* getdents_test_dir = "getdents";
    
    syscall_success(mkdir(getdents_test_dir, 0));
    syscall_success(chdir(getdents_test_dir));
    
    create_file_helper("tmp");
    syscall_success(fd = open("tmp", O_WRONLY, 0));
    syscall_fail(getdents(fd, dirents, 4 * sizeof(dirent_t)), ENOTDIR);
    syscall_success(close(fd));
    
    /* getdents works */
    syscall_success(mkdir("dir", 0777));
    syscall_success(mkdir("dir/1", 0777));
    create_file_helper("dir/2");
    syscall_success(mkdir("dir/1/3", 0777));
    create_file_helper("dir/3");
    
    syscall_success(fd = open("dir", O_RDONLY, 0));
    syscall_success(ret = getdents(fd, dirents, 4 * sizeof(dirent_t)));
    test_assert(4 * sizeof(dirent_t) == ret, NULL);
    
    syscall_success(ret = getdents(fd, dirents, sizeof(dirent_t)));
    test_assert(ret, NULL);
    
    syscall_success(ret = getdents(fd, dirents, sizeof(dirent_t)));
    test_assert(!ret, NULL);
    
    syscall_success(lseek(fd, 0, SEEK_SET));
    file_pos_helper(fd, 0);
    syscall_success(ret = getdents(fd, dirents, 5 * sizeof(dirent_t)));
    test_assert(5 * sizeof(dirent_t) == ret, NULL);
    syscall_success(ret = getdents(fd, dirents, sizeof(dirent_t)));
    test_assert(!ret, NULL);
    syscall_success(close(fd));
    
    syscall_success(chdir(".."));
    syscall_success(remove_recur(getdents_test_dir));
}

int s5fstest_main(int argc, char **argv)
{
    
    /* Initialize the test */
    test_init();
    
    /* Create the test directory */
    s5fstest_start();
    
    syscall_success(chdir(root));
    
    /* Test cases */
    s5fstest_stat();
    s5fstest_chdir();
    s5fstest_mkdir_rmdir();
    s5fstest_mknod();
    s5fstest_paths();
    s5fstest_close();
    s5fstest_unlink();
    s5fstest_fd();
    s5fstest_open_close();
    s5fstest_read();
    s5fstest_getdents();
    
    syscall_success(chdir("/"));
    
    /* Remove the directory */
    s5fstest_term();
    
    /* Finalize the test*/
    test_fini();
    
    return 0;
}