#include "stdafx.h"
#include "Pipeline.h"
#include "PipelineFactory.h"



DECLARE_LOG_SRC("Pipeline", LOGCAT_Common);

/*************************************************************/
PipelineStepParam::PipelineStepParam()
{
}

PipelineStepParam::PipelineStepParam(
	const QString& sName, 
	QVariant vValue,
	QVariant vMin,
	QVariant vMax)
{
	m_sParamName = sName;
	m_vParamVal = vValue;
	m_vMinVal = vMin;
	m_vMaxVal = vMax;
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


QVariant PipelineStepParam::MinValue() const
{
	return m_vMinVal;
}

QVariant PipelineStepParam::MaxValue() const
{
	return m_vMaxVal;
}


BEGIN_SERMIG_MAP(PipelineStepParam, 1, "PipelineStepParam")
	SERMIG_MAP_ENTRY(1)
END_SERMIG_MAP


void PipelineStepParam::SerializeV1(Archive& ar)
{
	if (ar.isStoring())
	{
		// Write
		ar.label("name") << m_sParamName;
		ar.label("val")  << m_vParamVal;
		ar.label("min")  << m_vMinVal;
		ar.label("max")  << m_vMaxVal;
		return;
	}

	// Read
	ar.label("name") >> m_sParamName;
	ar.label("val")  >> m_vParamVal;
	ar.label("min")  >> m_vMinVal;
	ar.label("max")  >> m_vMaxVal;
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

	for(int i = 0; i< m_listParams.count(); ++i)
		m_mapParamPositions[m_listParams.at(i).Name()] = i;
}

QString PipelineStep::Name() const
{
	return m_sName;
}

const QList<PipelineStepParam>& PipelineStep::Params() const
{
	return m_listParams;
}

QList<PipelineStepParam>& PipelineStep::Params()
{
	return m_listParams;
}

PipelineData PipelineStep::Process(const PipelineData& input)
{
	return m_funcOp(input, m_listParams);
}


bool PipelineStep::ContainsParam(const QString& sName) const
{
	return m_mapParamPositions.contains(sName);
}


void PipelineStep::SetParamVal(const QString& sName, const QVariant& vVal)
{
	Q_ASSERT(m_mapParamPositions.contains(sName));
	int iIdx = m_mapParamPositions.value(sName);
	m_listParams[iIdx].SetValue(vVal);
}

void PipelineStep::Dump() const
{
	QMapIterator<QString, int> iter(m_mapParamPositions);
	LOGINFO("m_mapParamPositions");
	while (iter.hasNext())
	{
		iter.next();
		LOGINFO("  %s=%d", qPrintable(iter.key()), iter.value());
	}
}


BEGIN_SERMIG_MAP(PipelineStep, 2, "PipelineStep")
	SERMIG_MAP_ENTRY(2)
	SERMIG_MAP_ENTRY(1)
END_SERMIG_MAP


void PipelineStep::SerializeV1(Archive& ar)
{
	// Read
	ar.label("Name") >> m_sName;

	// Restore from name
	PipelineStep ps = PipelineFactory::CreateStep(m_sName);

	// Copy it
	*this = ps;
}

void PipelineStep::SerializeV2(Archive& ar)
{
	if (ar.isStoring())
	{
		// Write
		ar.label("Name") << m_sName;

		// Write the parameter values
		ar << m_listParams;
		return;
	}

	// Read
	ar.label("Name") >> m_sName;
	QList<PipelineStepParam> listParams;
	ar >> listParams;

	// Restore from name
	PipelineStep ps = PipelineFactory::CreateStep(m_sName);

	// Only restore the values from what we serialized
	if (listParams.count() == ps.Params().count())
	{
		for (const PipelineStepParam& psp : listParams)
		{
			if (ps.ContainsParam(psp.Name()))
				ps.SetParamVal(psp.Name(), psp.Value());
		}
	}

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


QList<cv::UMat> Pipeline::Process(const cv::UMat& inputImg)
{
	PipelineData input;
	input.img = inputImg;
	return Process(input);
}

QList<cv::UMat> Pipeline::Process(const PipelineData& input)
{
	// Collect all results in an array
	QList<cv::UMat> listOuts;

	// Process each step
	PipelineData inputCpy = input;
	for (int i = 0; i < count(); ++i)
	{
		inputCpy = (*this)[i].Process(inputCpy);
		listOuts += inputCpy.img;
	}

	return listOuts;
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




