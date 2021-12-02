#pragma once

#include <QAbstractTableModel>
#include "Pipeline.h"


class PipelineTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	PipelineTableModel(QObject* parent);
	~PipelineTableModel();
	void SetPipeline(Pipeline* pPipeline);

	// Read access to the model
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	Qt::ItemFlags flags(const QModelIndex& index) const;

private:
	Pipeline* m_pPipeline = nullptr;
};
