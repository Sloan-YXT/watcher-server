#include <cstdio>
#include <iostream>
using namespace std;
#define FDEBUG(F, X, ...)                   \
    do                                      \
    {                                       \
        \  
    FILE *p = fopen(F, "a");                \
        fprintf(p, "in line %d", __LINE__); \
        fprintf(p, X, ##__VA_ARGS__);       \
        fprintf(p, "\n");                   \
        fclose(p);                          \
    } while (0)
int main(void)
{
    FDEBUG("face-transfer.log", "send err:%s", "aaaaaa");
}