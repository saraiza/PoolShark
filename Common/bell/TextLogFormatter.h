/*****************************************************
TextLogFormatter

	This is not a full logger, but merely a text log
	message formatter.
*****************************************************/
#pragma once
#include <QTimeZone>
#include "Logging.h"

namespace Logging {



class TextLogFormatter : public ILogger
{
public:
	TextLogFormatter();
	TextLogFormatter(ILog *pInnerLog);
	void Start() override;
	virtual QString GetLogFileName() override;
	virtual void Hint(Hints hint) override;

	virtual void Log(
                        int iLevel,
                        Qt::HANDLE threadId,
                        const char* pszSource,
                        const char* pszCategory,
                        const char* pszMsg);
        virtual void Log(
                        int iLevel,
                        Qt::HANDLE threadId,
                        const char* pszSource,
                        const char* pszCategory,
                        const QString& msg) override;
	void SetLog(ILog *pInnerLog);

	static const char* GetLevelString(int iLevel);

protected:
	ILog *m_pInnerLog = nullptr;
	virtual QString Timestamp();
};

class TextLogFormatterWTimeZone : public TextLogFormatter
{
public:
    TextLogFormatterWTimeZone();
    TextLogFormatterWTimeZone(ILog *pInnerLog);

    void setTimeZone(const QTimeZone tzone);

protected:
    virtual QString Timestamp() override;

private:
    QTimeZone m_tzone = QTimeZone::systemTimeZone();
    QString m_sTZAbbrev = QString("Z");
};

} // namespace


