//
// Created by user on 2021/11/28.
//

#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "lite_file.h"
#include "lite_memory_methods.h"
#include "lite_defs.h"

static lite_mutex unixBigLock = {PTHREAD_MUTEX_INITIALIZER};
struct unixInodeInfo *unixInodeInfoList = NULL;


static int unixFileLock(unixFile *file, struct flock *lock) {
//    return 0;
    int status = fcntl(file->fd,F_SETLK, lock);
    if (status)
        file->iLastError = errno;
    return status;
}

int findInodeInfo(unixFile *pFile, unixInodeInfo **out_ppInode) {
    assert(pFile && out_ppInode);
    struct stat fileStat;
    int rc = ((int (*)(char *, struct stat *)) unixSyscall[4].pCurrent)(pFile->name, &fileStat);
    *out_ppInode = NULL;

    lite_mutex_enter(&unixBigLock);
    struct unixInodeInfo *target = unixInodeInfoList;
    if (target) {
        do {
            if (fileStat.st_dev == target->fileId.dev && fileStat.st_ino == target->fileId.ino) {
                *out_ppInode = target;
                break;
            }
            target = target->next;
        } while (target != unixInodeInfoList);
    }

    if (*out_ppInode == NULL) {
        unixInodeInfo *inodeInfo = lite_malloc(sizeof(struct unixInodeInfo));
        memset(inodeInfo, 0, sizeof(struct unixInodeInfo));
        if (inodeInfo == NULL)
            return LITE_NO_MEM;
        inodeInfo->fileId.ino = fileStat.st_ino;
        inodeInfo->fileId.dev = fileStat.st_dev;
        inodeInfo->next = inodeInfo;
        inodeInfo->prev = inodeInfo;
        inodeInfo->pMutex = lite_mutex_alloc(LITE_MUTEX_FAST);
        if (unixInodeInfoList) {
            inodeInfo->next = unixInodeInfoList;
            inodeInfo->prev = unixInodeInfoList->prev;
            inodeInfo->prev->next = inodeInfo;
            inodeInfo->next->prev = inodeInfo;
            unixInodeInfoList = inodeInfo;
        } else {
            unixInodeInfoList = inodeInfo;
        }
        *out_ppInode = unixInodeInfoList;
    }
    (*out_ppInode)->nRef++;
    lite_mutex_leave(&unixBigLock);
    return LITE_SUCCESS;
}

static int unixLock(unixFile *file, int eFileLock) {
    assert(file && eFileLock <= EXCLUSIVE_LOCK);
    assert(eFileLock != PENDING_LOCK);
    if (file->eFileLock >= eFileLock)
        return LITE_SUCCESS;
    unixInodeInfo *inode = file->pInode;
    int status = LITE_SUCCESS;
    struct flock lock;
    lock.l_len = 1;
    lock.l_whence = SEEK_SET;
    lock.l_start = PENDING_BYTE;

    lite_mutex_enter(inode->pMutex);

    if (inode->eFileLock != file->eFileLock &&
        (inode->eFileLock >= PENDING_LOCK || eFileLock > SHARED_LOCK)) {
        status = LITE_BUSY;
        goto end_lock;
    }

    if (eFileLock == SHARED_LOCK && (inode->eFileLock == SHARED_LOCK || inode->eFileLock == RESERVED_LOCK)) {
        file->eFileLock = SHARED_LOCK;
        inode->nShared++;
        inode->nLock++;
        goto end_lock;
    }

    if (eFileLock == SHARED_LOCK || (inode->eFileLock <= RESERVED_LOCK && eFileLock == EXCLUSIVE_LOCK)) {
        if(eFileLock == SHARED_LOCK)
            assert(inode->eFileLock == NONE_LOCK && inode->nLock == 0 && inode->nShared == 0);
        lock.l_type = ((eFileLock == SHARED_LOCK) ? F_RDLCK : F_WRLCK);
        lock.l_start = PENDING_BYTE;
        if (unixFileLock(file, &lock)) {
            status = LITE_ERROR_LOCK;
            goto end_lock;
        }
    }

    if (eFileLock == SHARED_LOCK) {
        assert(inode->eFileLock == NONE_LOCK);
        assert(inode->nShared == 0 && inode->nLock == 0);
        lock.l_start = SHARED_BYTE;
        if (unixFileLock(file, &lock)) {
            status = LITE_ERROR_LOCK;
            goto end_lock;
        }

        lock.l_start = PENDING_BYTE;
        lock.l_type = F_UNLCK;
        if (unixFileLock(file, &lock)) {
            status = LITE_ERROR_LOCK;
            printf("unlock failed when lock shared!\n");
            goto end_lock;
        }
        file->eFileLock = SHARED_LOCK;
        inode->eFileLock = SHARED_LOCK;
        inode->nShared++;
        inode->nLock++;
    }
    else if (eFileLock == EXCLUSIVE_LOCK && inode->nShared > 1) {
        status = LITE_BUSY;
        goto end_lock;
    }
    else{
        lock.l_type = F_WRLCK;
        if(eFileLock == RESERVED_LOCK) {
            lock.l_start = RESERVED_BYTE;
            if(unixFileLock(file, &lock))
            {
                status = LITE_ERROR_LOCK;
                goto end_lock;
            }
        }
        if (eFileLock == EXCLUSIVE_LOCK) {
            lock.l_start = SHARED_BYTE;
            if (unixFileLock(file, &lock))
                status = LITE_ERROR_LOCK;
        }

    }
    if(status == LITE_SUCCESS)
    {
        file->eFileLock = eFileLock;
        inode->eFileLock = eFileLock;
    }
    else if(eFileLock == EXCLUSIVE_LOCK){
        file->eFileLock = PENDING_LOCK;
        inode->eFileLock = PENDING_LOCK;
    }
    end_lock:
    lite_mutex_leave(inode->pMutex);
    return status;
}

//eFileLock必须低于file->lockType
//eFileLock表示当前file锁被释放到什么级别
//排它锁只能被释放到NONE_LOCK
static int unixUnlock(unixFile *file, int eFileLock) {
    if (file->eFileLock <= eFileLock)
        return LITE_SUCCESS;
    int status = LITE_SUCCESS;
    struct unixInodeInfo *inode = file->pInode;
    struct flock lock;
    lock.l_whence = SEEK_SET;
    lite_mutex_enter(inode->pMutex);

    if (eFileLock == NONE_LOCK) {
        inode->nShared--;
        if(inode->nShared == 0) {
            lock.l_start = lock.l_len = 0;
            lock.l_type = F_UNLCK;
            if (unixFileLock(file, &lock))
                return LITE_ERROR_LOCK;
            inode->eFileLock = NONE_LOCK;
        }
        file->eFileLock = NONE_LOCK;
        inode->nLock--;
    }

    if (file->eFileLock > SHARED_LOCK) {
        assert(file->eFileLock == inode->eFileLock);
        if (eFileLock == SHARED_LOCK) {
            lock.l_type = F_RDLCK;
            lock.l_start = SHARED_BYTE;
            lock.l_len = 1;
            if(unixFileLock(file, &lock))
                return LITE_ERROR_LOCK;
        }
        lock.l_start = PENDING_BYTE;
        lock.l_len = 2;
        lock.l_type = F_UNLCK;
        if(unixFileLock(file, &lock))
            return LITE_ERROR_LOCK;
        file->eFileLock = inode->eFileLock = eFileLock;
    }

    lite_mutex_leave(inode->pMutex);
    return status;
}

static void unixClose(unixFile *file) {
    assert(file);
    struct unixUnusedFd *fd = lite_malloc(sizeof(struct unixUnusedFd));
    unixInodeInfo *inode = file->pInode;
    fd->fd = file->fd;
    fd->flag = 0;
    lite_mutex_enter(inode->pMutex);
    fd->next = inode->pUnusedFd;
    inode->pUnusedFd = fd;
    lite_mutex_leave(inode->pMutex);
    lite_free(file->name);
    lite_free(file);
}

static off_t unixSeek(unixFile *file, off_t offset, int whence) {
    return ((off_t (*)(int, off_t, int)) unixSyscall[12].pCurrent)(file->fd, offset, whence);
}

static size_t unixRead(unixFile *file, void *buffer, size_t size) {
    unixInodeInfo *inode = file->pInode;
    return ((int (*)(int, void *, size_t)) unixSyscall[8].pCurrent)(file->fd, buffer, size);
}

static size_t unixWrite(unixFile *file, void *buffer, size_t size) {
    unixInodeInfo *inode = file->pInode;
    return ((int (*)(int, void *, size_t)) unixSyscall[9].pCurrent)(file->fd, buffer, size);
}

static int unixFsync(unixFile *file) {
    return ((int (*)(int)) unixSyscall[13].pCurrent)(file->fd);
}

static long unixFileSize(unixFile *file) {
    struct stat fileStat;
    ((int (*)(char *, struct stat *)) unixSyscall[4].pCurrent)(file->name, &fileStat);
    return fileStat.st_size;
}

lite_io_methods unix_io_methods = {1, unixClose, unixRead, unixWrite, unixSeek, unixLock, unixUnlock, unixFsync,
                                   unixFileSize};

const lite_io_methods *current_io_methods = &unix_io_methods;

