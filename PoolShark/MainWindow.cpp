#include "stdafx.h"
#include "MainWindow.h"
#include "PipelineFactory.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ParamWidgetFloat.h"
#include "ParamWidgetInt.h"
#include "ParamWidgetEnum.h"
#include <QStandardPaths>
#include "Cursor.h"

#include <opencv2/imgcodecs/imgcodecs.hpp>     // cv::imread()
#include <opencv2/core/cuda.hpp>


DECLARE_LOG_SRC("MainWindow", LOGCAT_Common);

#define MAX_RECENT_FILES		5

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

	// Setup the add step menu
	QMenu* pAddStepMenu = new QMenu(this);
	for (QString sClassName : PipelineFactory::StepNames())
	{
		QAction* pAction = pAddStepMenu->addAction(sClassName);
		pAction->setData(sClassName);
		VERIFY(connect(pAction, SIGNAL(triggered()), this, SLOT(OnAddStep())));
		//pAddStepMenu->addSeparator();
	}
	ui.pbAddStep->setMenu(pAddStepMenu);
	VERIFY(connect(ui.viewSteps->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::OnViewSteps_currentRowChanged));

	LoadConfig();

	UpdateControls();
}

MainWindow::~MainWindow()
{
}

void MainWindow::BuildRecentFilesMenu()
{
	if (ui.actionRecentFiles->menu())
		delete ui.actionRecentFiles->menu();

	QMenu* pMenu = new QMenu(this);
	for (QString sFilename : m_slRecentFiles)
	{
		QAction* pAction = pMenu->addAction(sFilename);
		pAction->setData(sFilename);
		VERIFY(connect(pAction, SIGNAL(triggered()), this, SLOT(OnOpenRecentFile())));
	}
	ui.actionRecentFiles->setMenu(pMenu);

}


void MainWindow::OnOpenRecentFile()
{
	QAction* pAction = dynamic_cast<QAction*>(sender());
	Q_ASSERT(pAction);
	QString sFilename = pAction->data().toString();

	if (!QFile::exists(sFilename))
	{
		int iIndex = m_slRecentFiles.indexOf(sFilename);
		m_slRecentFiles.removeAt(iIndex);
		LOGINFO("Removed recent file '%s'", qPrintable(sFilename));
		BuildRecentFilesMenu();
		return;
	}

	OpenFile(sFilename);
}

void MainWindow::SaveConfig()
{
	QString sConfigFilename = ConfigFilename();
	QFileInfo fi(sConfigFilename);
	QDir dir = fi.dir();
	dir.mkpath(dir.absolutePath());
	this->toFile(sConfigFilename, SerMig::Option::OPT_Text);
}

void MainWindow::LoadConfig()
{
	QString sConfigFilename = ConfigFilename();
	if (!QFileInfo::exists(sConfigFilename))
		return;

	try
	{
		this->fromFile(sConfigFilename);
	}
	catch (const Exception&)
	{
		LOGERR("Could not load config");
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	// Must save before we delete image windows
	SaveConfig();

	for (ImagesWindow* pImgWnd : m_listImageWindows)
	{
		if (pImgWnd)
			delete pImgWnd;
	}

	m_listImageWindows.clear();
}


QString MainWindow::ConfigFilename()
{
	QString sDir = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppConfigLocation);
	QDir dir(sDir);
	QString sFilename = dir.absoluteFilePath("PoolShark.sert");
	return sFilename;
}

void MainWindow::OnViewSteps_currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
	UpdateControls();
}

void MainWindow::UpdateControls()
{
	QModelIndexList mil = ui.viewSteps->selectionModel()->selectedIndexes();
	int iStepSelectionCount = mil.count();
	int iSelRow = ui.viewSteps->selectionModel()->currentIndex().row();
	//LOGINFO("iSelCount=%d iSelRow=%d", iStepSelectionCount, iSelRow);
	ui.pbRemoveStep->setEnabled(iStepSelectionCount > 0);
	ui.pbMoveStepUp->setEnabled(iStepSelectionCount == 1 && iSelRow > 0);
	ui.pbMoveStepDown->setEnabled(iStepSelectionCount == 1 && iSelRow <= m_pPipelineModel->rowCount() - 2);
	ui.pbApply->setEnabled(!ui.cbAutoApply->isChecked() && m_bParamsDirty);
}



void MainWindow::OnUnhandledException(ExceptionContainer exc)
{
	QString sMsg = exc.Msg();
	LOGERR("Unhandled Exception\n%s", qPrintable(sMsg));
	QMessageBox::warning(this, "Unhandled Exception", sMsg);
}

void MainWindow::OnAddStep()
{
	QAction* pAction = dynamic_cast<QAction*>(sender());
	Q_ASSERT(pAction);
	QString sOperationName = pAction->text();
	m_doc.pipeline += PipelineFactory::CreateStep(sOperationName);
	PipelineChanged();
}



void MainWindow::SetPipeline(const Pipeline& pipeline)
{
	m_doc.pipeline = pipeline;
	PipelineChanged();
}

void MainWindow::PipelineChanged()
{
	m_pPipelineModel->SetPipeline(&m_doc.pipeline);
	BuildParamWidgets();

	if(m_doc.sFilepath.isEmpty())
		setWindowTitle(m_sWindowTitle);
	else
		setWindowTitle(m_sWindowTitle + " - " + m_doc.sFilepath);

	UpdateControls();
	SetPipelineDirty();
}

void MainWindow::BuildParamWidgets()
{
	// Remove the spacer, we will add it back to the end
	// BUT DON'T DELETE IT!
	ui.paramsLayout->removeItem(ui.paramsSpacer);

	// Remove any existing child widgets
	for (QWidget* p : m_listSliders)
		delete p;
	m_listSliders.clear();

	// Iterate through all parameters in all steps.
	for (int iStep = 0; iStep < m_doc.pipeline.count(); ++iStep)
	{
		const PipelineStep& ps = m_doc.pipeline.at(iStep);

		for (int iParam = 0; iParam < ps.Params().count(); ++iParam)
		{
			QString sCookie = QString("%1:%2").arg(iStep).arg(iParam);
			const PipelineStepParam& psParam = ps.Params().at(iParam);
			QWidget* pW = CreateParamWidget(ps.Name(), psParam, sCookie);
			ui.paramsLayout->addWidget(pW);
			m_listSliders += pW;
		}
	}

	// Re-add the spacer to the end
	ui.paramsLayout->addItem(ui.paramsSpacer);
}


QWidget* MainWindow::CreateParamWidget(const QString& sStepName, const PipelineStepParam& psParam, const QVariant& vCookie)
{
	QString sParamFullName = QString("%1:%2").arg(sStepName, psParam.Name());

	switch (psParam.Type())
	{
	case QVariant::Int:
	{
		ParamWidgetInt* pW = new ParamWidgetInt(sParamFullName, psParam, vCookie, this);
		VERIFY(connect(pW, &ParamWidgetInt::ParamChanged, this, &MainWindow::OnParamChanged));
		return pW;
	}

	case QVariant::Double:
	{
		ParamWidgetFloat* pW = new ParamWidgetFloat(sParamFullName, psParam, vCookie, this);
		VERIFY(connect(pW, &ParamWidgetFloat::ParamChanged, this, &MainWindow::OnParamChanged));
		return pW;
	}

	case QVariant::StringList:
	{
		ParamWidgetEnum* pW = new ParamWidgetEnum(sParamFullName, psParam, vCookie, this);
		VERIFY(connect(pW, &ParamWidgetEnum::ParamChanged, this, &MainWindow::OnParamChanged));
		return pW;
	}

	default:
		Q_ASSERT(false);
	}

	return nullptr;
}


void MainWindow::SetPipelineDirty()
{
	if (ui.cbAutoApply->isChecked())
		ProcessPipeline();
	else
	{
		m_bParamsDirty = true;
		UpdateControls();
	}
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
	SetPipelineDirty();
}

void MainWindow::ProcessPipeline()
{
	WaitCursor wc;
	CreateImageWindows();

	// Rerun the entire pipeline for each input image
	for (int i = 0; i < m_listInputImages.count(); ++i)
	{
		// Run the pipeline
		QList<cv::UMat> listImages = m_doc.pipeline.Process(m_listInputImages.at(i));
		m_listImageWindows[i]->SetImages(listImages);
	}

	m_bParamsDirty = false;
	UpdateControls();
}

void MainWindow::on_pbApply_clicked()
{
	ProcessPipeline();
}

void MainWindow::on_cbAutoApply_clicked()
{
	UpdateControls();
}


void MainWindow::on_pbSelectInputs_clicked()
{
	QString sDir;
	QString sFilter("Images (*.png *.jpg)");

	QStringList sl = QFileDialog::getOpenFileNames(this, "Select Input Images", sDir, sFilter);
	if (sl.isEmpty())
		return;
	SetInputFiles(sl);
}

void MainWindow::SetInputFiles(QStringList slFiles)
{
	m_slInputFiles = slFiles;
	m_pInputsModel->setStringList(m_slInputFiles);

	// Load all the images
	m_listInputImages.clear();
	for (int i = 0; i < m_slInputFiles.count(); ++i)
	{
		cv::Mat img = cv::imread(qPrintable(m_slInputFiles.at(i)));
		m_listInputImages += img.getUMat(cv::ACCESS_READ);
	}

	ProcessPipeline();
}

void MainWindow::CreateImageWindows()
{
	// We might need to grow or shrink the number of windows
	int iImageCount = m_listInputImages.count();

	m_listImageWindows.resize(iImageCount);
	
	// Initialize empty slots
	for (int i = 0; i < iImageCount; ++i)
	{
		if (nullptr != m_listImageWindows[i])
			continue;	// Skip it, it already exists
		
		ImagesWindow* pWnd = new ImagesWindow(m_slInputFiles.at(i), nullptr);
		m_listImageWindows[i] = pWnd;
		pWnd->show();
		VERIFY(connect(pWnd, &ImagesWindow::Closing, this, &MainWindow::OnImagesWindowClosing));		
	}
}

void MainWindow::OnImagesWindowClosing()
{
	ImagesWindow* pWnd = dynamic_cast<ImagesWindow*>(sender());
	
	// Find the window in the list and zero it out
	for (int i = 0; i < m_listImageWindows.count(); ++i)
	{
		if (m_listImageWindows.at(i) == pWnd)
		{
			m_listImageWindows[i] = nullptr;
			return;
		}
	}
}

void MainWindow::on_actionNew_triggered()
{
	Pipeline pipeline;
	pipeline.SetName("Test");

	m_doc.bDirty = true;
	m_doc.sFilepath.clear();
	SetPipeline(pipeline);
}



void MainWindow::on_pbRemoveStep_clicked()
{
	QModelIndexList mil = ui.viewSteps->selectionModel()->selectedIndexes();
	Q_ASSERT(!mil.isEmpty());

	// Get row list
	QList<int> listIndexes;
	for (QModelIndex mi : mil)
		listIndexes += mi.row();
	std::sort(listIndexes.begin(), listIndexes.end());

	// Remove from the end
	while (!listIndexes.isEmpty())
	{
		int iIdx = listIndexes.last();
		m_doc.pipeline.removeAt(iIdx);
		listIndexes.removeLast();
	}

	PipelineChanged();

	// Restore the selection
	if (mil.count() == 1)
		ui.viewSteps->selectionModel()->select(mil.first(), QItemSelectionModel::SelectCurrent);
}

void MainWindow::on_pbMoveStepUp_clicked()
{
	QModelIndexList mil = ui.viewSteps->selectionModel()->selectedIndexes();
	Q_ASSERT(1 == mil.count());
	int iRow = mil.first().row();
	int iNewRow = iRow - 1;
	Q_ASSERT(iRow > 0);
	PipelineStep ps = m_doc.pipeline.takeAt(iRow);
	m_doc.pipeline.insert(iNewRow, ps);
	PipelineChanged();

	// Keep the same item selected
	QModelIndex miNewSel = m_pPipelineModel->index(iNewRow, mil.first().column());
	ui.viewSteps->selectionModel()->select(miNewSel, QItemSelectionModel::SelectCurrent);
}

void MainWindow::on_pbMoveStepDown_clicked()
{
	QModelIndexList mil = ui.viewSteps->selectionModel()->selectedIndexes();
	Q_ASSERT(!mil.isEmpty());
	int iRow = mil.first().row();
	int iNewRow = iRow + 1;
	Q_ASSERT(iRow < m_pPipelineModel->rowCount()-1);
	PipelineStep ps = m_doc.pipeline.takeAt(iRow);
	m_doc.pipeline.insert(iNewRow, ps);
	PipelineChanged();

	// Keep the same item selected
	QModelIndex miNewSel = m_pPipelineModel->index(iNewRow, mil.first().column());
	ui.viewSteps->selectionModel()->select(miNewSel, QItemSelectionModel::SelectCurrent);
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

	OpenFile(sFilepath);
	m_doc.sFilepath = sFilepath;

	m_slRecentFiles += sFilepath;
	if (m_slRecentFiles.count() > MAX_RECENT_FILES)
		m_slRecentFiles.removeLast();
	BuildRecentFilesMenu();
}


void MainWindow::OpenFile(const QString& sFilename)
{
	Pipeline pipeline;
	pipeline.fromFile(sFilename);
	m_doc.sFilepath = sFilename;
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



BEGIN_SERMIG_MAP(MainWindow, 1, "MainWindow")
	SERMIG_MAP_ENTRY(1)
END_SERMIG_MAP


void MainWindow::SerializeGeometry(Archive& ar, QWidget* pW)
{
	if (ar.isStoring())
	{
		Qt::WindowStates ws = windowState();
		ar << (int)ws;
		if (Qt::WindowMaximized != ws)
			ar << saveGeometry();	// I don't get it, will have to clean this up.
		else
		{
			// Maximized, handle it differently. What screen are we on?
			ar << geometry();
		}

		return;
	}

	Q_ASSERT(ar.isLoading());
	Qt::WindowStates ws = ar.ReadEnum<Qt::WindowStates>();
	setWindowState(ws);

	if (Qt::WindowMaximized != ws)
		restoreGeometry(ar.ReadByteArray());
	else
	{
		QRect rc;
		ar >> rc;
		setGeometry(rc);
		showMaximized();
	}
}


void MainWindow::SerializeV1(Archive& ar)
{
	if (ar.isStoring())
	{
		SerializeGeometry(ar, this);

		ar << m_slInputFiles;
		for (ImagesWindow* pWnd : m_listImageWindows)
		{
			bool bHasWnd = (bool)(pWnd != nullptr);
			ar << bHasWnd;
			if (pWnd)
				SerializeGeometry(ar, pWnd);
		}

		ar << m_slRecentFiles;
		ar << ui.cbAutoApply->isChecked();

		return;
	}

	Q_ASSERT(ar.isLoading());
	SerializeGeometry(ar, this);

	ar >> m_slInputFiles;
	SetInputFiles(m_slInputFiles);

	// The image window list should match the input files now
	for(int i = 0; i < m_listImageWindows.count(); ++i)
	{
		ImagesWindow* pWnd = m_listImageWindows[i];
		Q_ASSERT(pWnd);
		bool bHasWnd = ar.ReadBool();
		if (bHasWnd)
			SerializeGeometry(ar, pWnd);
		else
		{
			delete pWnd;
			m_listImageWindows[i] = nullptr;
		}
	}

	ar >> m_slRecentFiles;
	BuildRecentFilesMenu();

	ui.cbAutoApply->setChecked(ar.ReadBool());
}
