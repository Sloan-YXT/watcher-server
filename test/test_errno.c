#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
int main(void)
{
    int res = open("asdasdsdas", O_RDONLY);
    printf("errno = %d,n=%d\n", errno, res);
    res = open("test_file.cpp", O_RDONLY);
    printf("errno=%d,n=%d\n", errno, res);
}