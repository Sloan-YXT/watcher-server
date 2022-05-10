#include "../faceDetect.h"
#include <iostream>
int main(void)
{
    int n = faceDetect("/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml", "/home/yaoxuetao/yaoxuetao's stm32f103/photos/Wed May 11 06:02:46 202", "/home/yaoxuetao/yaoxuetao's stm32f103/faces/Wed May 11 06:02:46 202.jpg");
    cout << n << endl;
}