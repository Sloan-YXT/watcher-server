#include <fstream>
#include <iostream>
using namespace std;
int main(void)
{
    ifstream in; //
    in.open("data.conf", ios::in);
    double a, b, c, d, e;
    try
    {

        in >> a >> b >> c >> d;
        cout << a << b << c << d << endl;
        in.exceptions(ios::badbit | ios::eofbit);
        in >> e;
        cout << e << endl;
    }
    catch (const std::exception &e) // interesting:without ref delimites runtime multiscale
    {
        cout << e.what() << endl;
    }
}