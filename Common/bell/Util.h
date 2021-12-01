#pragma once
#include <QString>
#include <QMapIterator>
#include "Defs.h"
#include <qmath.h>
#include <QDir>
#include <QMetaMethod>

class QMimeData;
class IProgressSink;

/**
@brief Utility class for assorted things that are handy.

Everyone on the team should get to know the silly little static methods in this
class.

*/


class Util
{
public:
    /// Convert to a hex string: "0x0B"
    static QString toHex8(uchar val);

    /// Convert to a hex string: "0x0BCD"
    static QString toHex16(ushort val);

    /// Convert to a hex string: "0x0FACABCD"
    static QString toHex32(uint val);

    /// Sames as toHex8, but no '0x' in the beginning
    static QString toRawHex8(uchar val);

    /// Sames as toHex16, but no '0x' in the beginning
    static QString toRawHex16(ushort val);

    /// Construct a Memory Dump string
    static QString MemDump(const uchar *pMem, uint count, uint wordsPerLine = 4);

    /// Construct a Memory Dump string
	static QString MemDump(const ushort *pMem, uint count, uint wordsPerLine = 4, ushort usBaseAddr = 0xFFFF);

    /// Construct a Memory Dump string
    static QString MemDump(const short *pMem, uint count, uint wordsPerLine = 4);

    /// Construct a Memory Dump string
    static QString Dump(const float *pVals, uint count, uint wordsPerLine = 4);

    static QString Dump(const double *pVals, uint count, uint wordsPerLine = 4);


    /// Convert a date/time into a fixed-length timestamp that can be used in a filename.
    static QString toFilenameTimestamp(const QDateTime& dt);

    /// Simple YYYYMMDDThhmmss
    static QString toFilenameTimestampPlain(const QDateTime& dt);

    /// Recursively search a directory tree for files.
    static QFileInfoList RecursiveDirSearch(const QString& dirPath, const QStringList & nameFilters = QStringList("*"), QDir::Filters dirFilters = QDir::NoFilter, QDir::SortFlags sort = QDir::NoSort );

    /// Get the total size of all the files in a directory.
    static qint64 DirectorySize(const QString& dirPath, const QStringList & nameFilters = QStringList("*"), QDir::Filters dirFilters = QDir::NoFilter );

    /// Get the total size of free space in a directory
    static qint64 VolumeFreeSpace(const QString& dirPath);

    /// Get the total size of space in a folder
    static qint64 VolumeMaxSpace(const QString& dirPath);

    /// Force the directory to exist
    static void ForcePath(const QString& sDirOrFilePath);

	static void FlushFile(const QString& sFilename, bool bFlushDir = false);

	static void FlushFile(const QFileDevice * pFile);


	static QMap<QMetaMethod::MethodType, QStringList> PrintSignalsAndSlots(const QMetaObject * pMetaObject);

    static QString readAllText(QString filename);
    static QByteArray readAll(QString filename);
	static void writeAll(QByteArray baData, QString sFilename);
	static void toFile(QString sFilename, QString sContents);
	static void toFile(QString sFilename, QByteArray baContents);
	static QByteArray fromFile(QString sFilename);

    static QByteArray LoadResource(QString resPath);

    static void msleep(uint ms);



    static QStringList TrimStringList(const QStringList& sl);
    static QStringList ToLower(const QStringList& sl);
};


/**
@brief Handy class for ensuring that a bool untoggles in a function.

Use this on the stack to unset the value when the function leaves scope.
*/
class BoolSet
{
public:
    BoolSet(bool *pBool, bool bNewState)
    {
        m_bResetState = !bNewState;
        m_pb = pBool;
        *m_pb = bNewState;
    }
    ~BoolSet()
    {
        *m_pb = m_bResetState;
    }
private:
    bool m_bResetState;
    bool *m_pb = nullptr;
};


/**
@brief Handy class for ensuring that a bool untoggles in a function.

Use this on the stack to set a bool value on the stack and restore it to
its previous value when leaving.
*/
class BoolSetAndRestore
{
public:
    BoolSetAndRestore(bool *pBool, bool bNewState)
    {
        m_bResetState = *pBool;
        m_pb = pBool;
        *m_pb = bNewState;
    }
    ~BoolSetAndRestore()
    {
        *m_pb = m_bResetState;
    }
private:
    bool m_bResetState;
    bool *m_pb = nullptr;
};

/// Configure the thread name of the current thread.  The thread name will show up in the Visual Studio Debugger
/// for the desktop build. Very handy.
static void SetDebugThreadName(QString name);
