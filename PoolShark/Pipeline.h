#pragma once

#include <SerMig.h>
#include <opencv2/core/core.hpp>



/**
@brief Parameter for a pipeline step

Defines the type and name of a parameter.
*/
class PipelineStepParam
{
public:
	PipelineStepParam();
	PipelineStepParam(const QString& sName, QVariant vValue);
	QString Name() const;
	QVariant Value() const;
	void SetValue(const QVariant& vVal);

private:
	QString m_sParamName;
	QVariant m_vParamVal;
};



/**
@brief OpenCV image processing step

This class contains info about the parameters for the the operation,
the name, and the actual operation.
*/
class PipelineStep : public SerMig
{
public:
	DECLARE_SERMIG;
	using FuncOp = std::function<cv::Mat(const cv::Mat& img, const QList<PipelineStepParam>& listParams)>;

	PipelineStep();
	PipelineStep(const QString& sName, const QList<PipelineStepParam>& listParams, FuncOp funcOp);
	
	cv::Mat Process(const cv::Mat& imgInput);
	
	QString Name() const;
	QList<PipelineStepParam> Params() const;
	void SetParamVal(const QString& sName, const QVariant& vVal);

protected:
	// Derived class should call this
	void DefineParam(const QString& sName, const QVariant& vVal);

private:
	QString m_sName;
	QList<PipelineStepParam> m_listParams;
	QMap<QString, int> m_mapParamPositions;
	FuncOp m_funcOp;

	void SerializeV1(Archive& ar);
};
SERMIG_ARCHIVERS(PipelineStep)




/**
@brief OpenCV sequence of processing steps

*/
class Pipeline : public QList<PipelineStep>, public SerMig
{
public:
	DECLARE_SERMIG;
	Pipeline();
	QString Name() const;
	void SetName(const QString& sName);

private:
	QString m_sName;
	bool m_bDirty = true;

	void SerializeV1(Archive& ar);
};
SERMIG_ARCHIVERS(Pipeline)

