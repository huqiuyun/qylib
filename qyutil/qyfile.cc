#include "qyfile.h"

#ifdef WIN32
#include <direct.h>
#define GET_CURRENT_DIR _getcwd
#else
#include <unistd.h>
#define GET_CURRENT_DIR getcwd
#endif

#include <sys/stat.h>  // To check for directory existence.
#ifndef S_ISDIR  // Not defined in stat.h on Windows.
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)	/* block special */
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)	/* char special */
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)	/* directory */
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)	/* fifo or socket */
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)	/* regular file */
#define	S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)	/* symbolic link */
#define	S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)	/* socket */
#endif //S_ISDIR

#include <stdio.h>
#include <string.h>

DEFINE_NAMESPACE(qy)

bool QyFile::isFileExists(const char* filename)
{
	return(1 != isPathFile(filename)) ? false : true;
}

bool QyFile::isPathExists(const char* filename)
{
	return(0 != isPathFile(filename)) ? false: true;
}

bool QyFile::fileExists(const char* filename)
{
    struct stat info = {0};
    if (stat(filename, &info) != 0)
    {
        return false;
    }
    return S_ISREG(info.st_mode);
}

int QyFile::isPathFile(const char* filename)
{
    struct stat info = {0};
    if (stat(filename, &info) != 0)
    {
        return -1;
    }
    if (S_ISDIR(info.st_mode))
    {
        return 0;
    }
    if (S_ISREG(info.st_mode))
    {
        return 1;
    }
    return -2;
}

size_t QyFile::getFileSize(const char* filename)
{
    FILE* f = fopen(filename, "rb");
    size_t size = 0;
    if (f != NULL)
    {
        if (fseek(f, 0, SEEK_END) == 0)
        {
            size = ftell(f);
        }
        fclose(f);
    }
    return size;
}

END_NAMESPACE(qy)