#include "PipelineTableModel.h"

PipelineTableModel::PipelineTableModel(QObject* parent)
	: QAbstractTableModel(parent)
{

}

PipelineTableModel::~PipelineTableModel()
{

}

void PipelineTableModel::SetPipeline(Pipeline* pPipeline)
{
	beginResetModel();
	m_pPipeline = pPipeline;
	endResetModel();
}

// Read access to the model
int PipelineTableModel::rowCount(const QModelIndex& parent) const
{
	if (nullptr == m_pPipeline)
		return 0;

	return m_pPipeline->count();
}

int PipelineTableModel::columnCount(const QModelIndex& parent) const
{
	if (nullptr == m_pPipeline)
		return 0;
	return 1;
}


QVariant PipelineTableModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	{
		const PipelineStep& ps = m_pPipeline->at(index.row());
		switch (index.column())
		{
		case 0:
			return ps.Name();
		
		}
	}
	}

	return QVariant();
}

QVariant PipelineTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Orientation::Vertical)
		return section;
	else
	{
		switch (section)
		{
		case 0:
			return "Name";
		default:
			return "error";
		}
	}
}

Qt::ItemFlags PipelineTableModel::flags(const QModelIndex& index) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
