#include "stdafx.h"
#include "MainWindow.h"
#include "Application.h"
#include <QtWidgets/QApplication>
#include <iostream>                        // std::cout
#include <opencv2/core/core.hpp>           // cv::Mat
#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>     // cv::Canny()
#include <opencv2/highgui.hpp>
#include <QImage>
#include <Logging.h>
#include <Macros.h>

using namespace cv;

DECLARE_LOG_SRC("main", LOGCAT_Common);


int main(int argc, char *argv[])
{
    Application a(argc, argv);
    MainWindow w;
    VERIFY(a.connect(&a, &Application::UnhandledException, &w, &MainWindow::OnUnhandledException));
    w.show();
/*
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
    */
    LOGINFO("Pool Shark is alive!");

    return a.exec();
}
