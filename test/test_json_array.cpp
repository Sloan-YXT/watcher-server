#include <nlohmann/json.hpp>
#include <cstdio>
#include <iostream>
#include <array>
using namespace nlohmann;
using namespace std;
int main(void)
{
    json p = R"({"path":[1,2,3,4]})"_json;
    array<int, 4> arr = p["path"];
    cout << arr[2] << endl;
    vector<string> a;
    a.push_back("1");
    a.push_back("2");
    a.push_back("3");
    json q;
    q["data"] = a;
    cout << p.dump() << endl;
}