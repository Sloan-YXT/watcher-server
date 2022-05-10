#include <dirent.h>
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
using namespace std;
int main(void)
{
    DIR *face_dir = opendir("/home/yaoxuetao/yaoxuetao's raspi/faces");
    vector<string> files;
    while (1)
    {
        struct dirent *face_d = readdir(face_dir);
        if (face_d == NULL)
            break;
        string tmp = face_d->d_name;
        // cout << tmp << endl;
        //  int pos = tmp.find_last_of('/');
        //  int npos = tmp.length();
        //  cout << pos << endl;
        //  cout << npos - pos - 1 << endl;
        files.push_back(tmp);
    }
    for (string p : files)
    {
        cout << p << endl;
    }
}