#include <QMetaType>
#include "ChecksumDevice.h"
#include <stdio.h>
#include "SerMig.h"
#include "Progress.h"
#include "ArchiveBinaryV1.h"
#include "ArchiveTextV1.h"
#include "Logging.h"


DECLARE_LOG_SRC("Archive", LOGCAT_Common)



Archive* Archive::Create(QIODevice *pDevice, SerMig::Options options)
{
	QIODevice::OpenMode mode = pDevice->openMode();
	bool bStoring = (QIODevice::WriteOnly & mode) > 0;

	// We ALWAYS auto-detect when loading
	if (!bStoring)
	{
		// Auto-detect the format by trying each type until one matches
		Archive *pAr = new ArchiveTextV1(pDevice, options);
		pAr->m_options = options;
		if (pAr->Start())
			return pAr;
		delete pAr;

		// Last one is binary
		pAr = new ArchiveBinaryV1(pDevice, options);
		if (pAr->Start())
			return pAr;
		EXERR("A745", "Unknown format type");
		return nullptr;
	}

	// No auto-detect, explicetly create the right type
	Archive *pAr;
	if (options.testFlag(SerMig::OPT_Text))
		pAr = new ArchiveTextV1(pDevice, options);
	else if (options.testFlag(SerMig::OPT_Binary))
		pAr = new ArchiveBinaryV1(pDevice, options);
	else
		pAr = new ArchiveBinaryV1(pDevice, options);

	pAr->Start();
	return pAr;
}
		


Archive* Archive::Clone(QIODevice *pStream) const
{
	Archive *pClone = Archive::Create(pStream, m_options);
	return pClone;
}



Archive::Archive(bool bStoring, SerMig::Options options)
{
	m_options = options;
	m_iNextID = 0;
#ifdef SERMIG_DEBUG_TAGS
	m_bUsingDebugTags = true;
#else
	m_bUsingDebugTags = false;
#endif

	m_bStoring = bStoring;
	if(options.testFlag(SerMig::OPT_DebugTags) && m_bStoring)
		m_bUsingDebugTags = true;
}



/**
@brief Read or write the debug tag.

Used by the constructors.
*/
void Archive::InitDebugTags()
{
	if (m_bStoring)
		*this << m_bUsingDebugTags;
	else
		*this >> m_bUsingDebugTags;
}

bool Archive::isStoring() const
{
	return m_bStoring;
}


bool Archive::isLoading() const
{
	return !m_bStoring;
}



int Archive::ReadInt()
{
	int iVal;
	*this >> iVal;
	return iVal;
}

uint Archive::ReadUInt()
{
	uint uiVal;
	*this >> uiVal;
	return uiVal;
}

size_t Archive::ReadSizeT()
{
	quint64 val;
	*this >> val;
	return val;
}

bool Archive::ReadBool()
{
	bool bVal;
	*this >> bVal;
	return bVal;
}

float Archive::ReadFloat()
{
	float fVal;
	*this >> fVal;
	return fVal;
}


QByteArray Archive::ReadByteArray()
{
	QByteArray ba;
	*this >> ba;
	return ba;
}

QString Archive::ReadString()
{
	QString s;
	*this >> s;
	return s;
}

QStringList Archive::ReadStringList()
{
	QStringList sl;
	*this >> sl;
	return sl;
}

double Archive::ReadDouble()
{
	double dVal;
	*this >> dVal;
	return dVal;
}

QDateTime Archive::ReadDateTime()
{
    QDateTime qdt;
    *this >> qdt;
    return qdt;
}

QDateTime Archive::ReadDateTimeQt4()
{
	char sz[9];
	this->ReadRawData(sz, 9);
	return QDateTime();
}

QVariant Archive::ReadVariant()
{
	QVariant val;
	*this >> val;
	return val;
}

void Archive::_DebugTagEnd(SerMig *pSerMig)
{
	if(!m_bUsingDebugTags)
		return;
	if (m_bStoring)
		DebugTag();
	else
	{
		bool bTagOk = DebugTag();
		if(!bTagOk)
		{
			//if(device()->atEnd())
			//	throw SerMig::ArchiveException(QString("File/Archive does not have enough data (while in %1)").arg(pSerMig->className()));

			QString msg(QString("R/W ser data count mismatch, fix V%2 serializer for %1").arg(pSerMig->className(), QString::number(pSerMig->m_formatVer)));
			throw SerMig::SerMigTagException(qPrintable(msg));
		}
	}
}


int Archive::AddObjW(SerMig *pObj)
{
	Q_ASSERT(m_bStoring);
	int id = ++m_iNextID;	// Yes, I know I said 'Next'
	m_objMapW[pObj] = id;
	return id;
}

void Archive::AddObjR(int id, SerMig *pObj)
{
	Q_ASSERT(!m_bStoring);
	m_objMapR[id] = pObj;
}



void Archive::WriteDObj(SerMig *pObj)
{
	// Special case for NULL, use id of zero!
	if(NULL == pObj)
	{
		*this << (int)0;
		return;
	}

	// Have we seen this object before?
	QMap<SerMig*, int>::const_iterator iter;
	iter = m_objMapW.find(pObj);
	if(m_objMapW.end() != iter)
	{
		// Yes! This is a duplicate object. Just write the ID
		*this << iter.value();
		return;
	}

	// This is a new object, save it in our map
	int id = AddObjW(pObj);

	// Write the ID followed by the object
	*this << id;

	// Write the object (class name and serialized object)
	*this << pObj->className();

	pObj->Serialize(*this);
}


SerMig* Archive::ReadDObj()
{
	// Read the object ID from the stream
	int id;
	*this >> id;

	if(0 == id)
	{
		// Special case, NULL! Just return NULL and we are done.
		return NULL;
	}

	// Have we seen this object before?
	QMap<int, SerMig*>::const_iterator iter;
	iter = m_objMapR.find(id);
	if(m_objMapR.end() != iter)
	{
		// Yes, so it is a duplicate pointer.
		// Just return the object
		return iter.value();
	}

	// No, we have not seen this object before. Build it!

	// Grab the class name
	QString className = this->ReadString();
	Q_ASSERT(!className.isEmpty());

	// Create an instance of the class (by name)
	SerMig *pNewObj = DynObj::Create<SerMig>(className);
	if(NULL == pNewObj)
		throw SerMig::ArchiveException(QString("Could not create instance of type '%1'").arg(className));

	// Save this object in our map
	AddObjR(id, pNewObj);

	// Deserialize the data into our new object
	pNewObj->Serialize(*this);

	return pNewObj;
}

