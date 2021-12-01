#include "LogEntry.h"
#include "SerMig.h"


using namespace Logging;

BEGIN_SERMIG_MAP(LogEntry, 2, "LogEntry")
	SERMIG_MAP_ENTRY(2)
	SERMIG_MAP_ENTRY(1)
END_SERMIG_MAP


void LogEntry::SerializeV1(Archive& ar)
{

	Q_ASSERT(!ar.isStoring());
	ar >> iLevel;
	ar >> timeStamp;
	uint uiThreadId;
	ar >> uiThreadId;
	quint64 ui64 = uiThreadId;
	threadId = (Qt::HANDLE)ui64;
	QString s;
	ar.ReadString(); //  ar >> pszSource;
	ar.ReadString(); // ar >> pszCategory;
	ar >> msg;
}


void LogEntry::SerializeV2(Archive& ar)
{
	if(ar.isStoring())
	{
		ar << iLevel;
		ar << timeStamp;
		ar << (quint64)threadId;
		//ar << pszSource;
		//ar << pszCategory;
		ar << msg;
		return;
	}

	Q_ASSERT(!ar.isStoring());
	ar >> iLevel;
	ar >> timeStamp;
	quint64 ui;
	ar >> ui;
	threadId = (Qt::HANDLE)ui;
	//ar >> pszSource;
	//ar >> pszCategory;
	ar >> msg;
}
