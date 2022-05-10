#include <iostream>
#include <memory>
using namespace std;
struct A
{
    A()
    {
        cout << "A" << endl;
    }
    int a;
    ~A()
    {
        cout << "delete" << endl;
    }
};
struct B
{
    unique_ptr<A> a = unique_ptr<A>(new A());
    int b = 3;
};
int main(void)
{
    cout << "start" << endl;
    {
        B b;
        // cout << b.a->a << endl;
    }
    cout << "end" << endl;
}