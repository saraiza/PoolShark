#pragma once

#include <QWidget>
#include <opencv2/core/core.hpp>
#include "ui_ImagesWindow.h"

class ImagesWindow : public QWidget
{
	Q_OBJECT

public:
	ImagesWindow(const QString& sTitle, QWidget *parent = Q_NULLPTR);
	~ImagesWindow();

	void SetImages(const QList<cv::Mat>& listImages);

signals:
	void Closing();

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	Ui::ImagesWindow ui;
};
