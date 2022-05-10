#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
int main(void)
{
    double a = 1, b = 3.45;
    char a1[16], b1[16];
    sprintf(a1, "%16f", a);
    sprintf(b1, "%16f", b);
    cout << a << endl;
    cout << b << endl;
}