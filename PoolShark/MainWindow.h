#pragma once

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include "ui_MainWindow.h"
#include "Pipeline.h"
#include "PipelineTableModel.h"



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);

public slots:
    void OnUnhandledException(ExceptionContainer exc);

private slots:
    void OnParamChanged(QVariant vCookie, QVariant vNewValue);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_pbSelectInputs_clicked();

private:
    Ui::MainWindowClass ui;
    QString m_sWindowTitle;
    PipelineTableModel* m_pPipelineModel = nullptr;

    void SetPipeline(const Pipeline& pipeline);
    void BuildParamWidgets();

    struct {
        Pipeline pipeline; ///< The main user 'document'
        bool bDirty = false;
        QString sFilepath;
    } m_doc;

    QStringList m_slInputs;
    QStringListModel* m_pInputsModel;
};
