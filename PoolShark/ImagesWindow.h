#pragma once

#include <QWidget>
#include <opencv2/core/core.hpp>
#include "ui_ImagesWindow.h"
#include <QImage>

class ImagesWindow : public QWidget
{
	Q_OBJECT

public:
	ImagesWindow(const QString& sTitle, QWidget *parent = Q_NULLPTR);
	~ImagesWindow();

	void SetImages(const QList<cv::UMat>& listImages);

signals:
	void Closing();

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	Ui::ImagesWindow ui;
	void RebuildPanes(int iCount);
	void ClearGrid();
	QList<ImagePane*> m_listPanes;
};
