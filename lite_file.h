//
// Created by user on 2021/11/26.
//

#ifndef MULTITHREADEDFILEOPERATE_LITE_FILE_H
#define MULTITHREADEDFILEOPERATE_LITE_FILE_H
#include <unistd.h>
#include <sys/stat.h>
#include "Mutex.h"
#include "lite_syscall.h"
typedef struct unixFileId{
    __dev_t dev;
    __ino_t ino;
}unixFileId;
struct unixUnusedFd{
    int fd;
    int flag;
    struct unixUnusedFd *next;
};
typedef struct lite_file lite_file;
typedef struct lite_io_methods{
    int version;
    void (*xClose)(lite_file *file);
    long (*xRead)(lite_file *file,void *buffer,long size);
    long (*xWrite)(lite_file *file,const void *buffer,long size);
    long (*xLseek)(lite_file *file,long offset,int whence);
    int (*xLock)(lite_file *file,int eFileLock);
    int (*xUnlock)(lite_file *file,int eFileLock);
    int (*xFsync)(lite_file *file);
    long (*xFileSize)(lite_file *file);
}lite_io_methods;

struct lite_file{
    const lite_io_methods *pMethods;
};

typedef struct unixInodeInfo{
    unixFileId fileId;
    lite_mutex  *pMutex;
    long nRef;
    long nLock;
    long nShared;
    void *pAppData;
    unsigned char eFileLock;
    struct unixUnusedFd *pUnusedFd; /*unused fd to close*/
    struct unixInodeInfo *next;
    struct unixInodeInfo *prev;
}unixInodeInfo;


typedef struct lite_VFS{
    int iVersion;
    int maxNameLength;
    char *name;
    struct lite_VFS *next;
    void *pAppData;

    int (*xOpen)(struct lite_VFS *pVfs,const char *name,lite_file ** ppOutFile,int flag,int *pOutFlag);
    int (*xDelete)(struct lite_VFS *pVfs,const char *name,int syncDir);
    int (*xCurrentTime)(struct lite_VFS *pVfs);
}lite_VFS;

typedef struct unixFile{
    const lite_io_methods* pMethods;
    const lite_VFS * pVfs;

    char* name;
    struct unixInodeInfo *pInode;
    int fd;
    int iLastError;
    unsigned char eFileLock;
}unixFile;



int lite_open(const char *name,lite_file **ppOutFile,int flag,int *ppOutFlag);
void lite_close(lite_file *file);
int lite_read(lite_file *file,void *buffer,int size);
int lite_write(lite_file *file,void *buffer,int size);
int lite_lseek(lite_file *file,int offset,int whence);
int lite_fileSize(lite_file *file);
int lite_delete(const char *filename);

#endif //MULTITHREADEDFILEOPERATE_LITE_FILE_H
