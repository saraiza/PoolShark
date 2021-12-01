#pragma once
#include "Logging.h"
#include "CriticalSection.h"
#include "IODeviceThread.h"

constexpr uint LOGGING_DEFAULT_ROLLOVER_SIZE = 2000000;
constexpr uint LOGGING_DEFAULT_MX_ROLLOVER_SIZE = 10000000;


namespace Logging {

/**
@brief File output logger.

This log writes log messages to a file.
*/
class LogFile : public ILog
{
public:
	LogFile(bool bRollover = false, uint uiRolloverSize = LOGGING_DEFAULT_ROLLOVER_SIZE);
	virtual ~LogFile();
	void Open(const QString& sFilename);
	void Rollover();
	void Close();
	QString Filename() const;
    void Rotate(const QString& filename);   ///< close the log and reopen with a new name

	void Log(const QString& line) override;
	void Flush() override;
	void Start() override;
	
    virtual QString GetLogFileName() override;
	virtual void Hint(Hints hint) override;
	void SetRollover(bool bRollOver, uint uiRolloverSize);

private:
	CriticalSection m_cs;
	QTextStream m_txtStrm;
	IODeviceThread m_file;
	QString m_sFilename;
	bool m_bRollover = false;
	uint m_uiRolloverSize;
	Hints iScanningHint = Hints::NotScanning;
};


} // namespace
