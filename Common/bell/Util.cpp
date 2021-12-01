#include "Util.h"
#include "Progress.h"
#include "Defs.h"
#include <QMimeData>
#include <QRegularExpression>
#include <QDirIterator>
#include <QCoreApplication>
#include <thread>
#include <QThread>
#include <QFontDatabase>
#include <QPixmap>
#include "Logging.h"
#include "Macros.h"
#include <QFileDevice>
#include <QStorageInfo>
#include <QDateTime>

// 'type cast': conversion from 'int' to 'HANDLE' of greater size
// This is ok for the FlushFile function below.
#ifdef Q_OS_WIN
#pragma warning(disable : 4312)
#include <io.h>
#endif

// For SetDebugThreadName
//#include "Ver.h"

#ifndef Q_OS_WIN
#	include <unistd.h>
#	include <stdio.h>
#	include <string.h>
#	include <errno.h>
#	include <fcntl.h>
#	include <stdlib.h>
#	include <sys/sendfile.h>
#	include <sys/stat.h>
#else
#include <windows.h>
#endif

extern void SetDebugThreadName(QString sName);


DECLARE_LOG_SRC("Util", LOGCAT_Common)


QString Util::toHex8(uchar val)
{
	return QString("0x%1").arg(QString::number(val, 16), 2, '0');
}


QString Util::toHex16(ushort val)
{
	return QString("0x%1").arg(QString::number(val, 16), 4, '0');
}

QString Util::toRawHex8(uchar val)
{
	return QString("%1").arg(QString::number(val, 16), 2, '0');
}


QString Util::toRawHex16(ushort val)
{
	return QString("%1").arg(QString::number(val, 16), 4, '0');
}

QString Util::toHex32(uint val)
{
	return QString("0x%1").arg(QString::number(val, 16), 8, '0');
}

void Util::msleep(uint ms)
{
	QThread::msleep(ms);
}


QString Util::MemDump(const short *pMem, uint count, uint wordsPerLine)
{
	const ushort *pUsMem = (const ushort*)pMem;

	return MemDump(pUsMem, count, wordsPerLine);
}


QString Util::MemDump(const uchar *pMem, uint count, uint wordsPerLine)
{
    QString dump, line;
	uint uiLineSamp = 0;
	while(count--)
	{
		ushort samp = *pMem++;
		line.append(QString(" %1").arg(toRawHex8(samp)));
		if(++uiLineSamp >= wordsPerLine)
		{
			dump += line;
			dump += "\n";
			line.clear();
			uiLineSamp = 0;
		}
	}

	if(uiLineSamp)
		dump += line;

	return dump;
}


QString Util::MemDump(const ushort *pMem, uint count, uint wordsPerLine, ushort usBaseAddr)
{
	QString dump, line;
	const ushort *pPos = pMem;
	uint countTrace = count;
	uint uiLineSamp = 0;
	ushort usPos = 0;
	while(count--)
	{
		// first line?
		if ((0 == usPos % wordsPerLine) && (usBaseAddr != 0xFFFF))
			line.append(QString("%1:").arg(toRawHex16(usPos + usBaseAddr)));

		ushort samp = *pMem++;
		line.append(QString(" %1").arg(toRawHex16(samp)));
		if(++uiLineSamp >= wordsPerLine)
		{
			dump += line;
			dump += "\n";
			line.clear();
			uiLineSamp = 0;
		}

		++usPos;
	}

	if(uiLineSamp)
		dump += line;

	return dump.toUpper();
}

QString Util::Dump(const float *pVals, uint count, uint wordsPerLine)
{
	QString dump, line;
	uint countTrace = count;
	uint uiLineSamp = 0;
	while(count--)
	{
		QString sNum = QString(" %1").arg(QString::number(*pVals++));
		line.append(sNum);
		if(++uiLineSamp >= wordsPerLine)
		{
			dump += line;
			dump += "\n";
			line.clear();
			uiLineSamp = 0;
		}
	}

	if(uiLineSamp)
		dump += line;

	return dump;
}

QString Util::Dump(const double *pVals, uint count, uint wordsPerLine)
{
	QString dump, line;
	uint countTrace = count;
	uint uiLineSamp = 0;
	while(count--)
	{
		QString sNum = QString(" %1").arg(QString::number(*pVals++));
		line.append(sNum);
		if(++uiLineSamp >= wordsPerLine)
		{
			dump += line;
			dump += "\n";
			line.clear();
			uiLineSamp = 0;
		}
	}

	if(uiLineSamp)
		dump += line;

	return dump;
}

QString Util::toFilenameTimestamp(const QDateTime& dt)
{
	return dt.toString("yyyyMMdd-hhmmss");
}



QString Util::toFilenameTimestampPlain(const QDateTime& dt)
{
	return dt.toString("yyyyMMddThhmmss");
}

QFileInfoList Util::RecursiveDirSearch(const QString& dirPath, const QStringList & nameFilters, QDir::Filters dirFilters, QDir::SortFlags sort)
{
	// The directory we will search through
	QDir dir(dirPath);

	// Get the files in this directory
	QDir::Filters fileFilters = dirFilters;
	fileFilters |= QDir::Files;
	fileFilters &= ~QDir::Dirs;

	// The files list we will return
	QFileInfoList topFiles = dir.entryInfoList(nameFilters, fileFilters, sort);


	// Find the sub directories
	QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	// Recurse each directory
	foreach(QString subDir, subDirs)
	{
		subDir = dirPath + "/" + subDir;
		QFileInfoList subFiles = RecursiveDirSearch(subDir, nameFilters, dirFilters, sort);
		topFiles += subFiles;
	}

	return topFiles;
}

qint64 Util::DirectorySize(const QString& dirPath, const QStringList & nameFilters, QDir::Filters dirFilters)
{
	QFileInfoList logFiles = Util::RecursiveDirSearch(dirPath, nameFilters, dirFilters);
	qint64 dirSize = 0;
	foreach(QFileInfo fi, logFiles)
	{
		dirSize += fi.size();
	}
	return dirSize;
}

qint64 Util::VolumeMaxSpace(const QString& dirPath)
{
	QStorageInfo storage(dirPath);
	if(!storage.isValid())
		EXERR("DBRE", "Path %s is not valid", qPrintable(dirPath));

	LOGINFO("Checking max space for %s", qPrintable(storage.device()));
	return storage.bytesTotal();
}

qint64 Util::VolumeFreeSpace(const QString& dirPath)
{
	QStorageInfo storage(dirPath);
	if(!storage.isValid())
		EXERR("K6RD", "Path %s is not valid", qPrintable(dirPath));

	LOGDBG("Checking free space for %s", qPrintable(storage.device()));
	return storage.bytesAvailable();
}


void Util::ForcePath(const QString& sDirOrFilePath)
{
	QFileInfo fi(sDirOrFilePath);
	QString sPath = fi.absolutePath();
	QDir dir(sPath);
	dir.mkpath(".");
}


void Util::FlushFile(const QString& sFilename, bool bFlushDir /* = false*/)
{
#ifdef UNDER_EMBEDDED
	// Flush the file
	struct stat64 stat_buf;
	fstat64(sFilename.toLocal8Bit().constData(), &stat_buf);
	long long hFile = open64(sFilename.toLocal8Bit().constData(), O_WRONLY | O_CREAT, stat_buf.st_mode);
	fsync(hFile);
	close(hFile);

	// Optional directory flush
	if (bFlushDir)
	{
		QFileInfo fi(sFilename);
		QString sParentDir = fi.dir().absolutePath();
		fstat64(sParentDir.toLocal8Bit().constData(), &stat_buf);
		long long hDir = open64(sParentDir.toLocal8Bit().constData(), O_WRONLY | O_CREAT, stat_buf.st_mode);
		fsync(hDir);
		close(hDir);
	}
#endif
}

void Util::FlushFile(const QFileDevice * pFile)
{
	Q_ASSERT(pFile);
#ifdef Q_OS_WIN
	int fd = pFile->handle();
	HANDLE handle = (HANDLE)_get_osfhandle(fd);
	FlushFileBuffers(handle);
#else
	fsync(pFile->handle());
#endif
}



QString InstallFontDir()
{
	QString path = QCoreApplication::instance()->applicationDirPath();

	LOGINFO("Bin path = '%s'", qPrintable(path));

	QDir dir(path);

	if (!dir.cd("Fonts"))
	{
		LOGWRN("Could not find font folder in bin path");
		return "";
	}

	QString fontPath = dir.absolutePath();

	LOGINFO("font path = '%s'", qPrintable(fontPath));
	return dir.absolutePath();
}

QMap<QMetaMethod::MethodType, QStringList> Util::PrintSignalsAndSlots(const QMetaObject * pMetaObject)
{
	static QMap<QMetaMethod::MethodType, QString> methodCode {
		{  QMetaMethod::Signal, QString::number(QSIGNAL_CODE) },
		{  QMetaMethod::Slot, QString::number(QSLOT_CODE) },
		{  QMetaMethod::Method, QString::number(QMETHOD_CODE) }
	};

	static QMap<QMetaMethod::MethodType, QString> labels{
		{  QMetaMethod::Signal, "Signal" },
		{  QMetaMethod::Slot, "Slot" },
		{  QMetaMethod::Method, "Method" }
	};

	QMap<QMetaMethod::MethodType, QStringList> methods;
	int count = pMetaObject->methodCount();

	for (int i = 0; i < count; ++i)
	{
		const QMetaMethod mmTest = pMetaObject->method(i);
		QString sig = mmTest.methodSignature();
		if (sig.startsWith("destroyed") || sig.startsWith("objectNameChanged"))
			continue;

		auto type = mmTest.methodType();

		methods[type] << methodCode.value(type) + sig;
		QLOGINFO() << labels.value(type) + " " + methodCode.value(type) + sig;
	}
	return methods;
}

QByteArray Util::readAll(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return QByteArray();

	QByteArray ba = file.readAll();
	return ba;
}

void Util::writeAll(QByteArray baData, QString sFilename)
{
	QFile file(sFilename);
	if (!file.open(QIODevice::WriteOnly))
		EXERR("Could not open file '%s' for writing", qPrintable(sFilename));

	file.write(baData);
}


QString Util::readAllText(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return QString();

	QTextStream txtStr(&file);
	QString all = txtStr.readAll().trimmed();
	return all;
}

void Util::toFile(QString sFilename, QByteArray baContents)
{
	QFile file(sFilename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		EXERR("LFDY", "Failed to open file for writing: %s", qPrintable(sFilename));

	file.write(baContents);
	file.close();
}


QByteArray Util::fromFile(QString sFilename)
{
	QFile file(sFilename);
	if (!file.open(QFile::ReadOnly))
		EXERR("L4DY", "Failed to open file for reading: %s", qPrintable(sFilename));
	int iSize = file.size();

	QByteArray ba = file.read(iSize);
	file.close();
	return ba;
}

void Util::toFile(QString sFilename, QString sContents)
{
	QFile file(sFilename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		EXERR("LWKK", "Failed to open file for writing: %s", qPrintable(sFilename));

	file.write(qPrintable(sContents));
	file.close();
}


QByteArray Util::LoadResource(QString resPath)
{
	//LOGINFO("Loading file '%s'", qPrintable(resPath));
	QFile resFile(resPath);
	bool __BadResourcePath = resFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
	Q_ASSERT(__BadResourcePath);		// Yes, the name is inverted. But the text will show up in the assert
	QByteArray baRbf = resFile.readAll();
	return baRbf;
}


QStringList Util::TrimStringList(const QStringList& sl)
{
	QStringList slNew;
	foreach(const QString s, sl)
		slNew += s.trimmed();
	return slNew;
}


QStringList Util::ToLower(const QStringList& sl)
{
	QStringList slNew;
	foreach(const QString s, sl)
		slNew += s.toLower();
	return slNew;
}

