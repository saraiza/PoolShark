#pragma once

#include <SerMig.h>
#include <opencv2/core/core.hpp>



/**
@brief Parameter for a pipeline step

Defines the type and name of a parameter. This is designed sort of as a
half-assed union.
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
	PipelineStepParam(const QString& sName,
						const QStringList& slEnums);
	QString Name() const;

	QVariant::Type Type() const;

	QVariant Value() const;
	void SetValue(const QVariant& vVal);
	QVariant MinValue() const;
	QVariant MaxValue() const;

	// Enum handling
	QStringList EnumNames() const;
	QList<int> EnumValues() const;

private:
	QString m_sParamName;
	QVariant m_vParamVal;
	QVariant m_vMinVal;
	QVariant m_vMaxVal;
	
	struct {
		QStringList names;
		QList<int> values;
	} m_enum;

	void SerializeV1(Archive& ar);
	void SerializeV2(Archive& ar);
};
SERMIG_ARCHIVERS(PipelineStepParam)


struct PipelineData {
	cv::UMat img;

	std::vector<std::vector<cv::Point>> contours;
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
	using FuncOp = std::function<PipelineData(const PipelineData& input, const QList<PipelineStepParam>& listParams)>;

	PipelineStep();
	PipelineStep(const QString& sName, const QList<PipelineStepParam>& listParams, FuncOp funcOp);
	
	PipelineData Process(const PipelineData& input);
	
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

	QList<cv::UMat> Process(const cv::UMat& inputImg);
	QList<cv::UMat> Process(const PipelineData& input);

private:
	QString m_sName;
	bool m_bDirty = true;

	void SerializeV1(Archive& ar);
};
SERMIG_ARCHIVERS(Pipeline)

