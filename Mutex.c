//
// Created by user on 2021/11/26.
//

#include <malloc.h>
#include <assert.h>
#include "Mutex.h"
#include "lite_memory_methods.h"
lite_mutex *pthreadMutexAlloc(int type) {
    static lite_mutex staticMutex[1]={
            PTHREAD_MUTEX_INITIALIZER
    };
    if (type > 2)
        return NULL;
    switch (type) {
        case LITE_MUTEX_FAST:{
            lite_mutex *mutex = lite_malloc(sizeof(lite_mutex));
            if(mutex == NULL)
                return NULL;
            pthread_mutex_init(&mutex->mutex, NULL);
            return mutex;
        }
        case LITE_MUTEX_RECURSIVE: {
            lite_mutex *mutex = malloc(sizeof(lite_mutex));
            if(mutex == NULL)
                return NULL;
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mutex->mutex,&attr);
            pthread_mutexattr_destroy(&attr);
            return mutex;
        }
        default:{
            return &staticMutex[type - 2];
        }
    }
}

void pthreadMutexFree(lite_mutex *mutex){
    pthread_mutex_destroy(&mutex->mutex);
    lite_free(mutex);
}

int pthreadMutexEnter(lite_mutex *mutex){
    int rc = pthread_mutex_lock(&mutex->mutex);
    return rc;
}

int pthreadMutexTry(lite_mutex *mutex)
{
    int rc = pthread_mutex_trylock(&mutex->mutex);
    return rc;
}

int pthreadMutexLeave(lite_mutex *mutex)
{
    int rc = pthread_mutex_unlock(&mutex->mutex);
    return rc;
}
const lite_mutex_methods mutexMethods = {pthreadMutexAlloc,pthreadMutexFree,pthreadMutexEnter,pthreadMutexTry,pthreadMutexLeave};

const lite_mutex_methods* current_mutex_methods = &mutexMethods;



lite_mutex  *lite_mutex_alloc(int iType)
{
    return current_mutex_methods->MutexAlloc(iType);
}
void lite_mutex_free(lite_mutex *mutex)
{
    current_mutex_methods->MutexFree(mutex);
}
int lite_mutex_enter(lite_mutex *mutex)
{
    return current_mutex_methods->MutexEnter(mutex);
}
int lite_mutex_try(lite_mutex *mutex)
{
    return current_mutex_methods->MutexTry(mutex);
}
int lite_mutex_leave(lite_mutex *mutex)
{
    return current_mutex_methods->MutexLeave(mutex);
}








