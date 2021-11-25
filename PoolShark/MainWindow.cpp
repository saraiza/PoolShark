#include "stdafx.h"
#include "MainWindow.h"
#include <iostream>                        // std::cout
#include <opencv2/core/core.hpp>           // cv::Mat
#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>     // cv::Canny()
#include <opencv2/highgui.hpp>

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Experiment with OpenCV
    QString sFilename = "C:/dev/PoolShark.a/test/images/IMG_4430.jpg";
    Mat img;
    img = imread(qPrintable(sFilename));
    imshow("img", img);
}
