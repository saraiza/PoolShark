#include "stdafx.h"
#include "MainWindow.h"
#include "PipelineFactory.h"
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

}


void MainWindow::OnUnhandledException(ExceptionContainer exc)
{
	QString sMsg = exc.CurrentException().Msg();
	QMessageBox::warning(this, "Unhandled Exception", sMsg);
}

void MainWindow::on_actionNew_triggered()
{
    m_doc.pipeline = Pipeline();
	m_doc.bDirty = true;
	m_doc.sFilepath.clear();

	QStringList slNames = PipelineFactory::StepNames();
	for (QString s : slNames)
		m_doc.pipeline += PipelineFactory::CreateStep(s);
}

void MainWindow::on_actionOpen_triggered()
{
	QString sFilter = "ipl";
	QString sFilepath = QFileDialog::getOpenFileName(this,
		"Open Pipeline",
		m_doc.sFilepath,
		"Imaging Pipeline (*.ipl)",
		&sFilter);

	m_doc.pipeline.fromFile(sFilepath);
	m_doc.sFilepath = sFilepath;
	m_doc.bDirty = false;
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
}