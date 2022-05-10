#include <sys/epoll.h>
#include <cstdio>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
int epfd;
struct epoll_event event[10];

void *test(void *args)
{
    while (1)
    {
        printf("in epoll wait\n");
        int res = epoll_wait(epfd, event, 10, -1);
        printf("res=%d (%d %d)\n", res, EPOLLERR, EPOLLIN);
    }
}
int main(void)
{
    epfd = epoll_create(10);
    pthread_t a;
    setbuf(stdout, NULL);
    struct epoll_event b;
    // mkfifo("test.fifo", S_IRUSR | S_IWUSR | S_IRGRP);
    // b.events = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLERR;
    b.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP;
    struct sockaddr_in testSock;
    memset(&testSock, 0, sizeof(testSock));
    testSock.sin_family = AF_INET;
    testSock.sin_port = htons(6990);
    socklen_t len = sizeof(testSock);
    if (inet_aton("0.0.0.0", &testSock.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    int listenTest = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(listenTest, (struct sockaddr *)&testSock, sizeof(testSock)) == -1)
    {
        perror("error while trying to bind on portAdata");
        exit(1);
    }
    listen(listenTest, 100);
    struct sockaddr_in client_data;
    socklen_t client_data_addr_len = sizeof(client_data);
    int connfd = accept(listenTest, (struct sockaddr *)&client_data, &client_data_addr_len);
    b.data.fd = connfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &b) < 0)
    {
        perror("epoll ctl fail");
    }
    pthread_create(&a, NULL, test, NULL);
    sleep(3);
    // close(afd);
    // write(bfd, "test", 4);
    // close(bfd);
    // sleep(30);
    // epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
    // write(bfd, "test", 4);
    // close(connfd);
    while (1)
        ;
}