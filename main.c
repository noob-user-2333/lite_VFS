#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include "lite_file.h"
#include "lite_defs.h"

pthread_t thread1;
pthread_t thread2;
extern struct vfs unixVFS;
char buffer[1024];
void *test(void *arg)
{
    int status;
    lite_file *file,*file1;
    lite_open("/dev/shm/test.bin",&file,O_CREAT | O_RDWR |O_APPEND,0);
    for(int i=0;i<100;i++)
    {
        lite_write(file,"it is a good day!!\n",20);
        printf("runing %d times\n",i);
    }
    lite_close(file);
    return NULL;
}
pthread_t thread1,thread2,thread3;
int main() {
    printf("pid:%d\n",getpid());
//    test(NULL);
    pthread_create(&thread1, NULL,test,NULL);
    pthread_create(&thread2, NULL,test,NULL);
    pthread_create(&thread3, NULL,test,NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2,NULL);
    pthread_join(thread3,NULL);
    printf("process running succeed!!!\n");
    return 0;
}
