//
// Created by user on 2021/11/28.
//

#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "lite_syscall.h"

struct lite_syscall unixSyscall[]= {{"open_0",(lite_syscall_ptr)open,0},
                                    {"close_1",(lite_syscall_ptr)close,0},
                                    {"access_2",(lite_syscall_ptr)access,0},
                                    {"getcwd_3",(lite_syscall_ptr)getcwd,0},
                                    {"stat_4",(lite_syscall_ptr)stat,0},
                                    {"fstat_5",(lite_syscall_ptr)fstat,0},
                                    {"ftruncate_6",(lite_syscall_ptr)ftruncate,0},
                                    {"fcntl_7",(lite_syscall_ptr)fcntl,0},
                                    {"read_8",(lite_syscall_ptr)read,0},
                                    {"write_9",(lite_syscall_ptr)write,0},
                                    {"unlink_10",(lite_syscall_ptr)unlink,0},
                                    {"ioctl_11",(lite_syscall_ptr)ioctl,0},
                                    {"lseek_12",(lite_syscall_ptr)lseek,0},
                                    {"fsync_13",(lite_syscall_ptr)fsync,0}
                                    };