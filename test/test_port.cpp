#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstdlib>
int main(int argc, char *argv[])
{
    printf("%d\n", ntohs(atoi(argv[1])));
}