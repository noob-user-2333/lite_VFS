//
// Created by user on 2021/11/26.
//

#include <malloc.h>
#include <string.h>
#include <wait.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "lite_file.h"
#include "lite_defs.h"

extern lite_io_methods  *current_io_methods;
extern lite_VFS  *current_VFS;

int lite_open(const char *name,lite_file **ppOutFile,int flag,int *ppOutFlag)
{
    return current_VFS->xOpen(current_VFS,name,ppOutFile,flag,ppOutFlag);
}
void lite_close(lite_file *file)
{
    file->pMethods->xClose(file);
}
int lite_read(lite_file *file,void *buffer,int size)
{
    int status = file->pMethods->xLock(file,SHARED_LOCK);
    if(status)
        return status;
    file->pMethods->xRead(file,buffer,size);
    file->pMethods->xUnlock(file,NONE_LOCK);
    return 0;
}
int lite_write(lite_file *file,void *buffer,int size)
{
    again:
    if(file->pMethods->xLock(file,SHARED_LOCK) != LITE_SUCCESS)
    {
        file->pMethods->xUnlock(file,NONE_LOCK);
        goto again;
    }
    if(file->pMethods->xLock(file,RESERVED_LOCK) != LITE_SUCCESS)
    {
        file->pMethods->xUnlock(file,NONE_LOCK);
        goto again;
    }
    if(file->pMethods->xLock(file,EXCLUSIVE_LOCK) != LITE_SUCCESS){
        file->pMethods->xUnlock(file,NONE_LOCK);
        goto again;
    }
    int ret = file->pMethods->xWrite(file,buffer,size);
    file->pMethods->xUnlock(file,NONE_LOCK);
    return ret;
}
int lite_lseek(lite_file *file,int offset,int whence)
{
    return file->pMethods->xLseek(file,offset,whence);
}
int lite_fileSize(lite_file *file)
{
    return file->pMethods->xFileSize(file);
}
int lite_delete(const char *filename)
{
    return current_VFS->xDelete(current_VFS,filename,0);
}