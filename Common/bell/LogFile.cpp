#include "LogFile.h"
#include "Util.h"
#include <QDateTime>

DECLARE_LOG_SRC("LogFile", LOGCAT_Common);

Logging::LogFile::LogFile(bool bRollover, uint uiRolloverSize)  
	: m_bRollover(bRollover)
	, m_uiRolloverSize(uiRolloverSize)
	, m_file("LogFileIODev")
{
}

LogFile::~LogFile()
{
	Close();
}


void LogFile::SetRollover(bool bRollOver, uint uiRolloverSize)
{
	m_bRollover = bRollOver;
	m_uiRolloverSize = uiRolloverSize;
}

void LogFile::Open(const QString& sFilename)
{
	m_sFilename = sFilename;

    QFile *pQFile = new QFile(sFilename);

    if (!pQFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered))
    {
        delete pQFile;
        EXERR("YBX4", "Could not open file '%s'", qPrintable(sFilename));
    }

    m_file.Start(pQFile);
    m_txtStrm.setDevice(&m_file);

}

void LogFile::Rollover()
{
    // we should not get here if we aren't using rollover
	if (!m_bRollover)
	{
		EXERR("FBR2", "Rollover called when rollover is not enabled.");
	}

	if (m_file.isOpen())
		Close();
	QString sNewFileName = QString("log%1.txt").arg(Util::toFilenameTimestamp(QDateTime::currentDateTime()));
    QFileInfo fi{ m_sFilename };
    QString sDirname = fi.dir().absolutePath();
    fi.setFile(sDirname, sNewFileName);
    m_sFilename = fi.absoluteFilePath();
	QFile *pQFile = new QFile(m_sFilename);
	if (!pQFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Unbuffered))
	{
		delete pQFile;
		EXERR("FBR1", "Could not open file '%s'", qPrintable(m_sFilename));
	}
	m_file.Start(pQFile);
	m_txtStrm.setDevice(&m_file);
}

QString LogFile::GetLogFileName()
{
    return m_sFilename;
}

void LogFile::Close()
{
	m_txtStrm.flush();
	m_file.close();
}

void LogFile::Rotate(const QString& filename)
{
    CriticalSectionLocker lock(m_cs);

    // if we try to rotate to the same file, don't bother
    if (filename == m_sFilename)
    {
        return;
    }

    Close();
    Open(filename);
}

void LogFile::Start()
{
	// Make sure the file has been opened
	Q_ASSERT(!m_sFilename.isEmpty());
	Q_ASSERT(m_file.isOpen());
}

void LogFile::Log(const QString& line)
{
	CriticalSectionLocker lock(m_cs);
	Q_ASSERT(m_file.isOpen());
	uint iSize = m_file.size();
	if (iScanningHint != Hints::Scanning && m_bRollover && iSize > m_uiRolloverSize)
	{
		Rollover();
	}	
	m_txtStrm << line;
	m_txtStrm << '\n';
}

void LogFile::Flush()
{
	CriticalSectionLocker lock(m_cs);
	Q_ASSERT(m_file.isOpen());
	m_txtStrm.flush();
}

QString Logging::LogFile::Filename() const
{
	return m_sFilename;
}

void Logging::LogFile::Hint(Hints hint)
{
    LOGINFO("Setting hints to %d", hint);
	iScanningHint = hint;
}
