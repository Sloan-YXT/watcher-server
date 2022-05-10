#include <opencv4/opencv2/core/core.hpp>
#include <opencv4/opencv2/imgproc/imgproc.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <vector>
#include <cstdio>
#include "faceDetect.h"
using namespace std;
using namespace cv;
#define DEBUG
#ifdef DEBUG

#define DEBUG(X)                              \
    do                                        \
    {                                         \
        printf("debug:%d,%s\n", __LINE__, X); \
    } while (0)
#else
#define DEBUG(X)
#endif
int faceDetect(string data, string src, string dest)
{
    CascadeClassifier cascade;
    DEBUG(data.c_str());
    cascade.load(data);
    DEBUG("");
    Mat srcImage, grayImage, dstImage;
    cout << src << endl;
    DEBUG("");
    srcImage = imread(src);
    DEBUG("");
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
        imwrite(dest, dstImage);
    }
    return rect.size();
}