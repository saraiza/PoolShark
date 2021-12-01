#pragma once

//------------------------
//	INCLUDES
//------------------------

#include <QString>
#include <QScopedPointer>

//------------------------
//	FORWARD DECLARATIONS
//------------------------

class SystemDetailsPrivate;

//------------------------
//	CLASS SYSTEM DETAILS
//------------------------
class SystemDetails
{
public:

	static QString User();
	static QString Host();
	static QString OS();
	
	static QString BorgVersion();
	static QString GitUrl();
	static QString Branch();
	static QString RevisionDate();
	static QString Commit();
	static bool LocalMods();
	static bool IsDebug();
	static bool IsProduction();

	operator QString() const;

private:

	static QString GetUser();
	static bool GetIsDebug();
};
