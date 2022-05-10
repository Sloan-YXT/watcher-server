#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
pthread_t a;
int fd;
void *test(void *args)
{
    char a[20];
    while (1)
    {
        int n = recv(fd, &a, 20, MSG_WAITALL);
        sleep(10);
        printf("%d\n", n);
    }
}
int main(void)
{
    struct sockaddr_in testSock;
    memset(&testSock, 0, sizeof(testSock));
    testSock.sin_family = AF_INET;
    testSock.sin_port = htons(7777);
    socklen_t len = sizeof(testSock);
    if (inet_aton("192.168.1.85", &testSock.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(fd, (sockaddr *)&testSock, sizeof(testSock)))
    {
        perror("socket error");
        exit(1);
    }
    const char *b = "1234\n";
    if (send(fd, b, 5, 0) <= 0)
    {
        perror("shit:");
        exit(1);
    }
    pthread_create(&a, NULL, test, NULL);
    sleep(5);
    //经过测试,close不行，shutdown行
    shutdown(fd, SHUT_RDWR);
    close(fd);
    while (1)
        ;
}