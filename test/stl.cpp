#include <iostream>
#include <unordered_map>
using namespace std;
class A
{
public:
    A(A &)
    {
        cout << "&" << endl;
    }
    //离谱的是去掉这个构造函数通不过编译(应该是unordered_map要求实现这个构造函数!)
    A(const A &)
    {
        cout << "const &" << endl;
    }
    A(A &&)
    {
        cout << "&&" << endl;
    }
    A()
    {
    }
};
// insert copys,and emplace deprive
// library imprehensive!
int main(void)
{
    unordered_map<int, A> a;
    cout << "emplace" << endl;
    a.emplace(1, A());
    cout << "insert" << endl;
    a.insert({2, A()});
    cout << "insert" << endl;
    A b;
    a.insert({3, b});
    cout << "emplace(存在)" << endl;
    a.emplace(3, b);
    cout << "emplace(不存在)" << endl;
    a.emplace(4, b);
}