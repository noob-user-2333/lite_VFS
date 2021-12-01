cmake_minimum_required(VERSION 3.20)
project(MultithreadedFileOperate C)

set(CMAKE_C_STANDARD 11)

add_executable(MultithreadedFileOperate main.c Mutex.c Mutex.h lite_file.c lite_file.h lite_vfs.c
        lite_syscall.c lite_syscall.h lite_io_methods.c
        lite_memory_methods.c  lite_memory_methods.h lite_defs.h)
target_link_libraries(MultithreadedFileOperate PUBLIC pthread)