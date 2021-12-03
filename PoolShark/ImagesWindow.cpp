#include "ImagesWindow.h"

ImagesWindow::ImagesWindow(const QString& sTitle, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(sTitle);
}

ImagesWindow::~ImagesWindow()
{
}

void ImagesWindow::closeEvent(QCloseEvent* event)
{
	emit Closing();
}

QImage ImagesWindow::Mat2QImage(const cv::Mat& mat)
{
    QString sFilename = "C:/dev/PoolShark.a/test/images/IMG_4430.jpg";
    cv::Size sz = mat.size();
    const uchar* pData = mat.ptr();
    int iBytesPerLine = sz.width * 3;
    QImage qimg(pData, sz.width, sz.height, iBytesPerLine, QImage::Format::Format_BGR888);
    return qimg;
}
