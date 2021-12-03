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
