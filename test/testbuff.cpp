#include <iostream>
using namespace std;
int main(void)
{
    fclose(stdout);
    stdout = fopen("test.log", "a");
    setbuf(stdout, NULL);
    // cout不行,printf行！
    printf("test test test");
    while (1)
        ;
}