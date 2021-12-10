#pragma once

#include <SerMig.h>
#include <opencv2/core/core.hpp>



/**
@brief Parameter for a pipeline step

Defines the type and name of a parameter.
*/
class PipelineStepParam : public SerMig
{
public:
	DECLARE_SERMIG;
	PipelineStepParam();
	PipelineStepParam(const QString& sName,
					  QVariant vValue,
					  QVariant vMin, 
					  QVariant vMax);
	QString Name() const;

	QVariant Value() const;
	void SetValue(const QVariant& vVal);
	QVariant MinValue() const;
	QVariant MaxValue() const;

private:
	QString m_sParamName;
	QVariant m_vParamVal;
	QVariant m_vMinVal;
	QVariant m_vMaxVal;
	void SerializeV1(Archive& ar);
};
SERMIG_ARCHIVERS(PipelineStepParam)



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
	QList<PipelineStepParam>& Params();
	const QList<PipelineStepParam>& Params() const;
	bool ContainsParam(const QString& sName) const;
	void SetParamVal(const QString& sName, const QVariant& vVal);
	void Dump() const;

private:
	QString m_sName;
	QList<PipelineStepParam> m_listParams;	///< The actaul params are held in the list
	QMap<QString, int> m_mapParamPositions; ///< The map is for easy access by name
	FuncOp m_funcOp;

	void SerializeV2(Archive& ar);
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

	QList<cv::Mat> Process(const cv::Mat& img);

private:
	QString m_sName;
	bool m_bDirty = true;

	void SerializeV1(Archive& ar);
};
SERMIG_ARCHIVERS(Pipeline)

