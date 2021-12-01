//
// Created by user on 2021/11/26.
//

#ifndef MULTITHREADEDFILEOPERATE_MUTEX_H
#define MULTITHREADEDFILEOPERATE_MUTEX_H
#include <pthread.h>


#define LITE_MUTEX_FAST 0
#define LITE_MUTEX_RECURSIVE 1
#define LITE_MUTEX_STATIC_INODE 2





typedef struct lite_mutex{
    pthread_mutex_t mutex;
}lite_mutex;

typedef struct lite_mutexIOMethods{
    lite_mutex* (*MutexAlloc)(int type);
    void (*MutexFree)(lite_mutex* mutex);
    int (*MutexEnter)(lite_mutex* mutex);
    int (*MutexTry)(lite_mutex *mutex);
    int (*MutexLeave)(lite_mutex *mutex);
}lite_mutex_methods;

lite_mutex  *lite_mutex_alloc(int iType);
void lite_mutex_free(lite_mutex *mutex);
int lite_mutex_enter(lite_mutex *mutex);
int lite_mutex_try(lite_mutex *mutex);
int lite_mutex_leave(lite_mutex *mutex);

#endif //MULTITHREADEDFILEOPERATE_MUTEX_H
