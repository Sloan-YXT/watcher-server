#include <iostream>
#include <netinet/in.h>
using namespace std;
int main(void)
{
    int a, b;
    a = htonl(-1);
    b = ntohl(a);
    // 32760???这是怎么回事？
    printf("-1->%d\n", a);
    printf("-1->->%d\n", b);
}