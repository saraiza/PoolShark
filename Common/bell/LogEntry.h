#pragma once
#include "SerMig.h"
#include <QList>

namespace Logging
{
	// a structure to hold log information
	struct LogEntry : public SerMig
	{
		DECLARE_SERMIG;
		LogEntry() = default;
		LogEntry(int iLevel_,
				 QDateTime timeStamp_,
				 Qt::HANDLE threadId_,
				 const char* pszSource_,
				 const char* pszCategory_,
				 QString msg_)
		{
			iLevel = iLevel_;
			timeStamp = timeStamp_;
			threadId = threadId_;
			pszSource = pszSource_;
			pszCategory = pszCategory_;
			msg = msg_;
		}

		int iLevel;
		QDateTime timeStamp;
		Qt::HANDLE threadId;
		const char* pszSource;
		const char* pszCategory;
		QString msg;

	private:
		void SerializeV2(Archive& ar);
		void SerializeV1(Archive& ar);
	};

}

Q_DECLARE_METATYPE(Logging::LogEntry);
Q_DECLARE_METATYPE(QList<Logging::LogEntry>);