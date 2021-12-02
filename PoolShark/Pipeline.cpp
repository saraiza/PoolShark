#include "Pipeline.h"
#include "PipelineFactory.h"

/*************************************************************/
PipelineStepParam::PipelineStepParam()
{
}

PipelineStepParam::PipelineStepParam(const QString& sName, QVariant vValue)
{
	m_sParamName = sName;
	m_vParamVal = vValue;
}

QString PipelineStepParam::Name() const
{
	return m_sParamName;
}

QVariant PipelineStepParam::Value() const
{
	return m_vParamVal;
}

void PipelineStepParam::SetValue(const QVariant& vVal)
{
	m_vParamVal = vVal;
}

/*************************************************************/

PipelineStep::PipelineStep()
{

}

PipelineStep::PipelineStep(const QString& sName, const QList<PipelineStepParam>& listParams, FuncOp funcOp)
{
	m_sName = sName;
	m_listParams = listParams;
	m_funcOp = funcOp;
}

QList<PipelineStepParam> PipelineStep::Params() const
{
	return m_listParams;
}

cv::Mat PipelineStep::Process(const cv::Mat& imgInput)
{
	return m_funcOp(imgInput, m_listParams);
}


void PipelineStep::DefineParam(const QString& sName, const QVariant& vVal)
{
	int iIdx = m_listParams.count();
	m_listParams += PipelineStepParam(sName, vVal);
	Q_ASSERT(!m_mapParamPositions.contains(sName));
	m_mapParamPositions[sName] = iIdx;
}

void PipelineStep::SetParamVal(const QString& sName, const QVariant& vVal)
{
	Q_ASSERT(m_mapParamPositions.contains(sName));
	int iIdx = m_mapParamPositions.value(sName);
	m_listParams[iIdx].SetValue(vVal);
}


BEGIN_SERMIG_MAP(PipelineStep, 1, "PipelineStep") 
	SERMIG_MAP_ENTRY(1)	
END_SERMIG_MAP

void PipelineStep::SerializeV1(Archive& ar)
{
	if (ar.isStoring())
	{
		// Write
		ar.label("Name") << m_sName;
		return;
	}

	// Read
	ar.label("Name") >> m_sName;

	// Restore from name
	PipelineStep ps = PipelineFactory::CreateStep(m_sName);

	// Copy it
	*this = ps;
}
 

/*************************************************************/

Pipeline::Pipeline()
{
	m_sName = "Untitled";
}

QString Pipeline::Name() const
{
	return m_sName;
}

void Pipeline::SetName(const QString& sName)
{
	m_sName = sName;
}



BEGIN_SERMIG_MAP(Pipeline, 1, "Pipeline")
	SERMIG_MAP_ENTRY(1)
END_SERMIG_MAP

void Pipeline::SerializeV1(Archive& ar)
{
	if (ar.isStoring())
	{
		// Write
		ar.label("Name") << m_sName;
		ar.label("StepCount") << (int)this->count();
		for (PipelineStep& ps : *this)
			ar << ps;
		return;
	}

	// Read
	this->clear();
	ar.label("Name") >> m_sName;
	int iCount;
	ar.label("StepCount") >> iCount;
	while (iCount--)
	{
		PipelineStep ps;
		ar >> ps;
		this->append(ps);
	}
}




