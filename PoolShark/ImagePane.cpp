#include "stdafx.h"
#include "ImagePane.h"
#include <opencv2/core/core.hpp>           // cv::Mat
#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>     // cv::Canny()
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>

ImagePane::ImagePane(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    ui.label->setGeometry(QRect(4, 4, 50, 25));
    ui.label->setText("");
}

QImage ImagePane::Mat2QImage(const cv::Mat& mat)
{
    cv::Size sz = mat.size();
    const uchar* pData = mat.ptr();
    int iBytesPerLine = sz.width * 3;
    QImage qimg(pData, sz.width, sz.height, iBytesPerLine, QImage::Format::Format_BGR888);
    return qimg;
}

void ImagePane::resizeEvent(QResizeEvent* event)
{
    ui.wImage->setGeometry(QRect(0, 0, event->size().width(), event->size().height()));
    Refresh();
}


void ImagePane::Refresh()
{
    if (0 == m_originalImage.rows || 0 == m_originalImage.cols)
        return;

    // Scale the image to fit. Consider the aspect ratios.
    int iPaneWidth = this->size().width();
    int iPaneHeight = this->size().height();
    float fAspectPane = (float)iPaneHeight / (float)iPaneWidth;
    float fAspectImg = (float)m_originalImage.rows / (float)m_originalImage.cols;

    double dScale;
    int iCropWidth, iCropHeight;

    if (fAspectImg > fAspectPane)
    {
        // Image aspect is taller than pane aspect so height needs to be cropped to match the aspect ratio of the pane
        iCropWidth = m_originalImage.cols;
        iCropHeight = m_originalImage.cols * fAspectPane;
    }
    else
    {
        // Image is narrower and width needs to be cropped to match the aspect ratio of the pane
        iCropWidth = m_originalImage.rows / fAspectPane;
        iCropHeight = m_originalImage.rows;
    }

    dScale = (double)iPaneWidth / (double)iCropWidth;

    // Crop the image to the aspect ratio
    int iMarginLeft = (m_originalImage.cols - iCropWidth) / 2;
    int iMarginTop = (m_originalImage.rows - iCropHeight) / 2;
    cv::Mat imgCropped(m_originalImage, 
        cv::Range(iMarginTop, iMarginTop + iCropHeight),
        cv::Range(iMarginLeft, iMarginLeft + iCropWidth));

    // Now scale it
    cv::Size dsize(iPaneWidth, iPaneHeight);
    cv::Mat imgTarget;
    cv::resize(imgCropped, imgTarget, dsize, dScale, dScale);

    // Convert to a QImage and display it
    QImage qimg = Mat2QImage(imgTarget);
    ui.wImage->setPixmap(QPixmap::fromImage(qimg));
}



void ImagePane::Init(const QString& sLabel)
{
    ui.label->setText(sLabel);
}


void ImagePane::SetImage(const cv::Mat& matImg)
{
    m_originalImage = matImg;
    Refresh();
}

