#include <pthread.h>
#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
int main(void)
{
    printf("TestThread:%d\n", syscall(__NR_gettid));
    pthread_mutex_t lock;

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_lock(&lock);
    pthread_mutex_lock(&lock);
    // sleep(1);
    //  pthread_mutex_unlock(&lock);
    while (1)
        ;
}