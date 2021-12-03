#include "stdafx.h"
#include "MainWindow.h"
#include "PipelineFactory.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ParamWidgetFloat.h"
#include "ParamWidgetInt.h"

#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()


DECLARE_LOG_SRC("MainWindow", LOGCAT_Common);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_sWindowTitle = windowTitle();

	m_pPipelineModel = new PipelineTableModel(this);
	ui.viewSteps->setModel(m_pPipelineModel);


	m_pInputsModel = new QStringListModel(this);
	ui.viewInputs->setModel(m_pInputsModel);

	// Cleanup the examples
	delete ui.wFloatExample;
	ui.wFloatExample = nullptr;
	delete ui.wIntExample;
	ui.wIntExample = nullptr;
}


void MainWindow::OnUnhandledException(ExceptionContainer exc)
{
	QString sMsg = exc.CurrentException().Msg();
	QMessageBox::warning(this, "Unhandled Exception", sMsg);
}



void MainWindow::SetPipeline(const Pipeline& pipeline)
{
	m_doc.pipeline = pipeline;
	m_pPipelineModel->SetPipeline(&m_doc.pipeline);
	BuildParamWidgets();

	if(m_doc.sFilepath.isEmpty())
		setWindowTitle(m_sWindowTitle);
	else
		setWindowTitle(m_sWindowTitle + " - " + m_doc.sFilepath);
}

void MainWindow::BuildParamWidgets()
{
	// Remove the spacer, we will add it back to the end
	// BUT DON'T DELETE IT!
	ui.paramsLayout->removeItem(ui.paramsSpacer);

	// Remove any existing child widgets
	for (QObject* pObj : ui.paramsLayout->children())
	{
		QWidget* pW = dynamic_cast<QWidget*>(pObj);
		if (pW)
			delete pW;
	}

	// Iterate through all parameters in all steps.
	for (int iStep = 0; iStep < m_doc.pipeline.count(); ++iStep)
	{
		const PipelineStep& ps = m_doc.pipeline.at(iStep);

		for (int iParam = 0; iParam < ps.Params().count(); ++iParam)
		{
			const PipelineStepParam& psParam = ps.Params().at(iParam);
			QMetaType::fromType<float>();
			int iType = psParam.Value().type();
			bool bIntType = ((int)QMetaType::Int == iType);

			QString sCookie = QString("%1:%2").arg(iStep).arg(iParam);
			QString sParamFullName = QString("%1:%2").arg(ps.Name(), psParam.Name());
			if (bIntType)
			{
				ParamWidgetInt* pW = new ParamWidgetInt(this);
				pW->Init(sParamFullName, psParam, sCookie);
				VERIFY(connect(pW, &ParamWidgetInt::ParamChanged, this, &MainWindow::OnParamChanged));
				ui.paramsLayout->addWidget(pW);
			}
			else
			{
				ParamWidgetFloat* pW = new ParamWidgetFloat(this);
				pW->Init(sParamFullName, psParam, sCookie);
				VERIFY(connect(pW, &ParamWidgetFloat::ParamChanged, this, &MainWindow::OnParamChanged));
				ui.paramsLayout->addWidget(pW);
			}
		}
	}

	// Re-add the spacer to the end
	ui.paramsLayout->addItem(ui.paramsSpacer);
}


void MainWindow::OnParamChanged(QVariant vCookie, QVariant vNewValue)
{
	LOGINFO("OnParamChanged(%s, %s)",
		qPrintable(vCookie.toString()), qPrintable(vNewValue.toString()));

	// Decode the cookie, it contains two indexes into the tree
	QStringList sl = vCookie.toString().split(':');
	Q_ASSERT(sl.count() == 2);
	int iStep  = sl.at(0).toInt();
	int iParam = sl.at(1).toInt();

	// Go set the value
	m_doc.pipeline[iStep].Params()[iParam].SetValue(vNewValue);

	// Rerun the entire pipeline
	QString sFilename = "C:/dev/PoolShark.a/test/images/IMG_4430.jpg";
	cv::Mat img;
	img = cv::imread(qPrintable(sFilename));
	QList<cv::Mat> listImages = m_doc.pipeline.Process(img);
}



void MainWindow::on_pbSelectInputs_clicked()
{
	QString sDir;
	QString sFilter("Images (*.png *.jpg)");

	QStringList sl = QFileDialog::getOpenFileNames(this, "Select Input Images", sDir, sFilter);
	if (sl.isEmpty())
		return;
	m_slInputImageFiles = sl;
	m_pInputsModel->setStringList(m_slInputImageFiles);

	CreateImageWindows();
}


void MainWindow::CreateImageWindows()
{
	// Remove all existing windows
	for (ImagesWindow* pWnd : m_listImageWindows)
		delete pWnd;
	m_listImageWindows.clear();

	ImagesWindow* p = new ImagesWindow("test", this);
	p->showNormal();
	p->resize(800, 600);
	VERIFY(connect(p, &ImagesWindow::Closing, this, &MainWindow::OnImagesWindowClosing));
	return;


	// Generate a new window for each input file
	for (QString sFilename : m_slInputImageFiles)
	{
		ImagesWindow* pWnd = new ImagesWindow(sFilename, this);
		m_listImageWindows += pWnd;
		pWnd->show();
	}
}

void MainWindow::OnImagesWindowClosing()
{
	ImagesWindow* pWnd = dynamic_cast<ImagesWindow*>(sender());
}

void MainWindow::on_actionNew_triggered()
{
	// Build a test pipeline TEMPORARY!
	QStringList slNames = PipelineFactory::StepNames();
	Pipeline pipeline;
	for (QString s : slNames)
		pipeline += PipelineFactory::CreateStep(s);
	pipeline.SetName("Test");

	m_doc.bDirty = true;
	m_doc.sFilepath.clear();
	SetPipeline(pipeline);
}

void MainWindow::on_actionOpen_triggered()
{
	QString sFilter = "ipl";
	QString sFilepath = QFileDialog::getOpenFileName(this,
		"Open Pipeline",
		m_doc.sFilepath,
		"Imaging Pipeline (*.ipl)",
		&sFilter);

	if (sFilepath.isEmpty())
		return;

	Pipeline pipeline;
	pipeline.fromFile(sFilepath);
	m_doc.sFilepath = sFilepath;
	m_doc.bDirty = false;
	SetPipeline(pipeline);
}

void MainWindow::on_actionSave_triggered()
{
	if (m_doc.sFilepath.isEmpty())
		on_actionSaveAs_triggered();
	else
		m_doc.pipeline.toFile(m_doc.sFilepath);
}

void MainWindow::on_actionSaveAs_triggered()
{
	QString sFilter = "ipl";
	QString sFilepath = QFileDialog::getSaveFileName(this,
		"Save Pipeline",
		QString(),
		"Imaging Pipeline (*.ipl)",
		&sFilter);

	if (sFilepath.isEmpty())
		return;

	m_doc.pipeline.toFile(sFilepath);
	m_doc.sFilepath = sFilepath;
	m_doc.bDirty = false;
	setWindowTitle(m_sWindowTitle + " - " + m_doc.sFilepath);
}