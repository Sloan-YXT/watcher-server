#include "../cutil.h"
#include <iostream>
#include <unistd.h>
using namespace std;
int main(void)
{
    cout << "before del" << endl;
    sleep(3);
    rmAll("/home/yaoxuetao/CMPW-server/test/1");
    cout << "after del" << endl;
}