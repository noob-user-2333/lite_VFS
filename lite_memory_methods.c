//
// Created by user on 2021/11/28.
//

#include <assert.h>
#include <malloc.h>
#include "lite_memory_methods.h"

int lite_memoryInit(void *notUsed)
{
    return 0;
}

void lite_memoryShutdown(void *notUsed)
{
}

int lite_memoryRoundUp(int n)
{
    return ROUND8(n);
}

void *lite_memoryMalloc(int size)
{
    assert(size > 0);
    unsigned long *p = malloc(size + sizeof(unsigned long));
    if(p)
    {
        p[0] = size;
        p++;
    }
    return p;
}

void lite_memoryFree(void *p)
{
    p -= sizeof(unsigned long);
    free(p);
}
int lite_memorySize(void *p)
{
    unsigned long *ptr = p;
    assert(ptr != NULL);
    ptr--;
    return (int)ptr[0];
}

void *lite_memoryRealloc(void *p,int size)
{
    unsigned long * ptr = p;
    assert(ptr  && size > 0);
    assert(size == ROUND8(size));
    ptr--;
    ptr = realloc(p,size + 8);
    if(ptr)
    {
        ptr[0] = size;
        ptr++;
    }
    return ptr;
}

lite_memory_methods memoryMethods = {lite_memoryMalloc,lite_memoryFree,lite_memoryRealloc,lite_memorySize,
                                     lite_memoryInit,lite_memoryShutdown,lite_memoryRoundUp,NULL};


void *lite_malloc(int size)
{
    return  memoryMethods.xMalloc(size);
}

void lite_free(void* p)
{
    memoryMethods.xFree(p);
}

int lite_size(void *p)
{
    return memoryMethods.xSize(p);
}