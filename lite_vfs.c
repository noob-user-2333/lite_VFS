//
// Created by user on 2021/11/28.
//


#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include "lite_file.h"
#include "lite_defs.h"
#include "lite_memory_methods.h"

extern int findInodeInfo(unixFile *pFile, unixInodeInfo **out_ppInode);
extern const lite_io_methods *current_io_methods;
const lite_VFS  *current_VFS;


static int unixOpen(lite_VFS *pVfs,const char *name,lite_file **OutFile,int flag,int *pOutFlag)
{
    unsigned long  length = strlen(name);
    if(length > pVfs->maxNameLength)
        return LITE_NAME_TOO_LONG;
    unixFile * file = lite_malloc(sizeof(unixFile));
    file->name = lite_malloc(length + 1);
    memcpy(file->name,name,length);
    file->name[length] = 0;
    file->fd = open(file->name,flag, 0644);
    if(file->fd < LITE_MIN_FD)
    {
        lite_free(file->name);
        lite_free(file);
        return LITE_ERROR_OPEN;
    }
    findInodeInfo(file,&file->pInode);
    file->pVfs = current_VFS;
    file->pMethods = current_io_methods;
    *OutFile = file;
    return LITE_SUCCESS;
}

static int unixDelete(lite_VFS *pVfs,const char *name,int syncDir)
{
    return ((int (*)(const char*))unixSyscall[10].pCurrent)(name);
}


struct lite_VFS unixVFS = {1,1024,"unixVFS",NULL,NULL,unixOpen, unixDelete};
const lite_VFS  *current_VFS = &unixVFS;















