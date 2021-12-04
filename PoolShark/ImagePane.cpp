#include "stdafx.h"
#include "ImagePane.h"

ImagePane::ImagePane(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    ui.label->setGeometry(QRect(4, 4, 50, 25));
    ui.label->setText("");
}

void ImagePane::resizeEvent(QResizeEvent* event)
{
    ui.wImage->setGeometry(QRect(0, 0, event->size().width(), event->size().height()));
}

QImage ImagePane::Mat2QImage(const cv::Mat& mat)
{
    cv::Size sz = mat.size();
    const uchar* pData = mat.ptr();
    int iBytesPerLine = sz.width * 3;
    QImage qimg(pData, sz.width, sz.height, iBytesPerLine, QImage::Format::Format_BGR888);
    return qimg;
}


void ImagePane::Init(const QString& sLabel)
{
    ui.label->setText(sLabel);
}


void ImagePane::SetImage(const cv::Mat& matImg)
{
    QImage img = Mat2QImage(matImg);
    ui.wImage->setPixmap(QPixmap::fromImage(img));
}

