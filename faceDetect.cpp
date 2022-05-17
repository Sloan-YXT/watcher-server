#include <opencv4/opencv2/core/core.hpp>
#include <opencv4/opencv2/imgproc/imgproc.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <vector>
#include <cstdio>
#include "faceDetect.h"
#include "cutil.h"
using namespace std;
using namespace cv;
int faceDetect(string data, string src, string dest)
{

    CascadeClassifier cascade;
// DEBUG(data.c_str());
#ifdef DEBUG
    fclose(stderr);
    stderr = fopen("opencv.log", "a");
    setbuf(stderr, NULL);
#endif
    cascade.load(data);
    FTDEBUG("opencv.log", "normal", "src = %s,dest=%s", src.c_str(), dest.c_str());
    Mat srcImage, grayImage, dstImage;
    cout << src << endl;
    FDEBUG("opencv.log", "in opencv");
    srcImage = imread(src);
    FDEBUG("opencv.log", "in opencv ");
    dstImage = srcImage.clone();
    grayImage.create(srcImage.size(), srcImage.type());
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
    Scalar colors[] =
        {
            // 红橙黄绿青蓝紫
            CV_RGB(255, 0, 0),
            CV_RGB(255, 97, 0),
            CV_RGB(255, 255, 0),
            CV_RGB(0, 255, 0),
            CV_RGB(0, 255, 255),
            CV_RGB(0, 0, 255),
            CV_RGB(160, 32, 240)};
    vector<Rect> rect;
    cascade.detectMultiScale(grayImage, rect);
    for (int i = 0; i < rect.size(); i++)
    {
        Point center;
        int radius;
        center.x = cvRound((rect[i].x + rect[i].width * 0.5));
        center.y = cvRound((rect[i].y + rect[i].height * 0.5));

        radius = cvRound((rect[i].width + rect[i].height) * 0.25);
        circle(dstImage, center, radius, colors[i % 7], 2);
    }
    if (rect.size() != 0)
    {
        FDEBUG("opencv.log", "in opencv:face detected");
        imwrite(dest, dstImage);
    }
#ifdef DEBUG_MODE
    // debug模式不管花里胡哨监测不到
    if (rect.size() == 0)
    {
        imwrite(dest, dstImage);
        return 1;
    }
#endif
    return rect.size();
}