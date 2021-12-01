//
// Created by user on 2021/11/28.
//

#ifndef MULTITHREADEDFILEOPERATE_LITE_SYSCALL_H
#define MULTITHREADEDFILEOPERATE_LITE_SYSCALL_H

#include <fcntl.h>

typedef void (*lite_syscall_ptr)(void);

struct lite_syscall{
    const char *sName;
    lite_syscall_ptr pCurrent;
    lite_syscall_ptr pDefault;
};

extern struct lite_syscall unixSyscall[];
#endif //MULTITHREADEDFILEOPERATE_LITE_SYSCALL_H
