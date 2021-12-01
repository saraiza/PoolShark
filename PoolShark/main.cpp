#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <iostream>                        // std::cout
#include <opencv2/core/core.hpp>           // cv::Mat
#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>     // cv::Canny()
#include <opencv2/highgui.hpp>
#include <QImage>

using namespace cv;



QImage Mat2QImage(const cv::Mat& mat)
{
    QString sFilename = "C:/dev/PoolShark.a/test/images/IMG_4430.jpg";
    cv::Size sz = mat.size();
    const uchar* pData = mat.ptr();
    int iBytesPerLine = sz.width * 3;
    QImage qimg(pData, sz.width, sz.height, iBytesPerLine, QImage::Format::Format_BGR888);
    return qimg;
}




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // Experiment with OpenCV
    QString sFilename = "C:/dev/PoolShark.a/test/images/IMG_4430.jpg";
    Mat img;
    img = imread(qPrintable(sFilename));
    imshow("img", img);

    // Display in Qt
    QImage qimg = Mat2QImage(img);

    QLabel mLabel;
    mLabel.setPixmap(QPixmap::fromImage(qimg));
    mLabel.show();

    return a.exec();
}
