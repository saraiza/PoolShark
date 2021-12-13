#pragma once

#include <QtWidgets/QMainWindow>
#include <QStringListModel>
#include "ui_MainWindow.h"
#include "Pipeline.h"
#include "PipelineTableModel.h"
#include "ImagesWindow.h"
#include <SerMig.h>




class MainWindow : public QMainWindow, public SerMig
{
    Q_OBJECT
public:
    DECLARE_SERMIG;
    MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

public slots:
    void OnUnhandledException(ExceptionContainer exc);

private slots:
    void OnParamChanged(QVariant vCookie, QVariant vNewValue);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_pbSelectInputs_clicked();
    void OnImagesWindowClosing();
    void on_pbRemoveStep_clicked();
    void on_pbMoveStepUp_clicked();
    void on_pbMoveStepDown_clicked();
    void OnAddStep();
    void OnViewSteps_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_pbApply_clicked();
    void on_cbAutoApply_clicked();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindowClass ui;
    void UpdateControls();
    QString m_sWindowTitle;
    PipelineTableModel* m_pPipelineModel = nullptr;
    bool m_bParamsDirty = false;

    void SetPipeline(const Pipeline& pipeline);
    void PipelineChanged();
    void BuildParamWidgets();
    QList<QWidget*> m_listSliders;

    struct {
        Pipeline pipeline; ///< The main user 'document'
        bool bDirty = false;
        QString sFilepath;
    } m_doc;

    QStringListModel* m_pInputsModel;
    QStringList m_slInputFiles;
    void SetInputFiles(QStringList slFiles);
    QList<cv::Mat> m_listInputImages;

    QList<ImagesWindow*> m_listImageWindows;
    void CreateImageWindows();
    void ProcessPipeline();

    void SaveConfig();
    void LoadConfig();
    QString ConfigFilename();

    void SerializeGeometry(Archive& ar, QWidget* pW);
    void SerializeV1(Archive& ar);
};
