#include <iostream>
using namespace std;
void testF(int &&a)
{
    cout << "&&" << endl;
}
void testF(int &a)
{
    cout << "&" << endl;
}
void testF(const int &a)
{
    cout << "const &" << endl;
}
int main(void)
{
    int a = 1;
    testF(1);
    testF(a);
}