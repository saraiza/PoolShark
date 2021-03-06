#pragma once

#include <QWidget>
#include "ui_ImagePane.h"
#include <opencv2/core/core.hpp>

/**
@brief Show a single image in a region

This also shows an image number or whatever we want.
*/
class ImagePane : public QWidget
{
	Q_OBJECT

public:
	ImagePane(QWidget *parent = Q_NULLPTR);

	void Init(const QString& sLabel);
	void SetImage(const cv::UMat& img);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

private:
	Ui::ImagePane ui;

	cv::UMat m_originalImage;
	void Refresh();	///< Redraw the image
	QImage Mat2QImage(const cv::Mat& mat);
};
