#include <iostream>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;
int main(void)
{
    sockaddr_in ts;
    memset(&ts, 0, sizeof(ts));
    ts.sin_family = AF_INET;
    ts.sin_port = htons(20221);
    if (inet_aton("127.0.0.1", &ts.sin_addr) == 0)
    {
        perror("inet_aton failed");
        exit(1);
    }
    int listenTs = socket(AF_INET, SOCK_STREAM, 0);
    bind(listenTs, (struct sockaddr *)&ts, sizeof(ts));
    if (listen(listenTs, 10) < 0)
    {
        perror("listen failed");
        exit(1);
    }
    fd_set rd;
    FD_ZERO(&rd);
    FD_SET(listenTs, &rd);
    int maxfd = listenTs + 1;
    timeval tout = {0, 100};
    cout << tout.tv_usec << endl;
    select(maxfd, &rd, NULL, NULL, &tout);
    cout << tout.tv_usec << endl;
}