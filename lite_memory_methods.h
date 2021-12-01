//
// Created by user on 2021/11/28.
//

#ifndef MULTITHREADEDFILEOPERATE_LITE_MEMORY_METHODS_H
#define MULTITHREADEDFILEOPERATE_LITE_MEMORY_METHODS_H

#define ROUND8(n) ((n+7) & (~7))
typedef struct lite_memory_methods{
    void *(*xMalloc)(int);
    void (*xFree)(void *);
    void *(*xRealloc)(void *,int);
    int (*xSize)(void *);
    int (*xInit)(void*);
    void (*xShutdown)(void*);
    int (*xRoundUp)(int);
    void *pAppData;
}lite_memory_methods;

void *lite_malloc(int size);
void lite_free(void* p);
int lite_size(void *p);
extern lite_memory_methods memoryMethods;
#endif //MULTITHREADEDFILEOPERATE_LITE_MEMORY_METHODS_H
