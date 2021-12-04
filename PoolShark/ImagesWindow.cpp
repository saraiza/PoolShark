#include "stdafx.h"
#include "ImagesWindow.h"

ImagesWindow::ImagesWindow(const QString& sTitle, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(sTitle);
	ClearGrid();
}

ImagesWindow::~ImagesWindow()
{
}

void ImagesWindow::closeEvent(QCloseEvent* event)
{
	emit Closing();
}


void ImagesWindow::ClearGrid()
{
	// Empty the grid layout
	while (!ui.gridLayout->children().isEmpty())
	{
		QWidget* p = dynamic_cast<QWidget*>(ui.gridLayout->children().first());
		ui.gridLayout->removeWidget(p);
	}

	// Remove the grid
	delete ui.gridLayout;

	// Make a new grid
	ui.gridLayout = new QGridLayout(this);
	ui.gridLayout->setSpacing(1);
	ui.gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	ui.gridLayout->setContentsMargins(1, 1, 1, 1);

	m_listPanes.clear();
}


void ImagesWindow::RebuildPanes(int iCount)
{
	ClearGrid();

	// Calculate the row width. We will keep this simple for now
	// and assume we want the layout as square as possible.
	float fSide = qRound(qSqrt(iCount));
	int iRowSize = (int)fSide;

	// Rebuild the image panes to fit in the grid
	int iRow = 0;
	int iCol = 0;
	for (int i = 0; i<iCount; ++i)
	{
		ImagePane* pImgPane = new ImagePane(this);
		ui.gridLayout->addWidget(pImgPane, iRow, iCol, 1, 1);
		QString sLabel = QString("%1").arg(i + 1);
		pImgPane->Init(sLabel);

		m_listPanes += pImgPane;

		// Next
		if (++iCol >= iRowSize)
		{
			iCol = 0;
			++iRow;
		}
	}
}

void ImagesWindow::SetImages(const QList<cv::Mat>& listImages)
{
	if (listImages.count() != m_listPanes.count())
		RebuildPanes(listImages.count());

	for (int i = 0; i < listImages.count(); ++i)
		m_listPanes[i]->SetImage(listImages.at(i));
}
