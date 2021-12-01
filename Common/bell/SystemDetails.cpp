#include <SystemDetails.h>
#include <QHostInfo>
#include <QSysInfo>
#include <Ver.h>

//--------------------------------------------------------------------------------------------------
//	User ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::User()
{
	return GetUser();
}

//--------------------------------------------------------------------------------------------------
//	Host ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::Host()
{
	return QHostInfo::localHostName();
}

//--------------------------------------------------------------------------------------------------
//	OS ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::OS()
{
	return QSysInfo::prettyProductName();
}

//--------------------------------------------------------------------------------------------------
//	BorgVersion ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::BorgVersion()
{
	return Ver::APP_Version();
}

//--------------------------------------------------------------------------------------------------
//	GitUrl ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::GitUrl()
{
	return Ver::SVN_Url();
}

//--------------------------------------------------------------------------------------------------
//	Branch ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::Branch()
{
	return Ver::GetBranchFull();
}

//--------------------------------------------------------------------------------------------------
//	RevisionDate ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::RevisionDate() 
{
	return Ver::SVN_Revision_Date();
}

//--------------------------------------------------------------------------------------------------
//	RevisionText ( public )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::Commit()
{
	return Ver::GIT_Revision_Text();
}

//--------------------------------------------------------------------------------------------------
//	LocalMods ( public )
//--------------------------------------------------------------------------------------------------
bool SystemDetails::LocalMods()
{
	return Ver::SVN_Revision_IsLocalMod();
}

//--------------------------------------------------------------------------------------------------
//	IsDebug ( public )
//--------------------------------------------------------------------------------------------------
bool SystemDetails::IsDebug()
{
	return GetIsDebug();
}

//--------------------------------------------------------------------------------------------------
//	IsProduction ( public )
//--------------------------------------------------------------------------------------------------
bool SystemDetails::IsProduction()
{
	return Ver::IsProduction();
}

//--------------------------------------------------------------------------------------------------
//	operator QString ( public )
//--------------------------------------------------------------------------------------------------
SystemDetails::operator QString() const
{
	QString details;
	details.append(QString("SYSTEM DETAILS:\n"));
	details.append(QString("    Host         : %1\n").arg(Host()));
	details.append(QString("    OS           : %1\n").arg(OS()));
	details.append(QString("    User         : %1\n").arg(User()));
	details.append(QString("\n"));
	details.append(QString("SOFTWARE DETAILS:\n"));
	details.append(QString("    Borg Version : %1\n").arg(BorgVersion()));
	details.append(QString("    Revision Date: %1\n").arg(RevisionDate()));
	details.append(QString("    Branch       : %1\n").arg(Branch()));
	details.append(QString("    Commit       : %1\n").arg(Commit()));
	details.append(QString("    Git Url      : %1\n").arg(GitUrl()));
	details.append(QString("    Local changes: %1\n").arg(LocalMods() ? "YES" : "NO"));
	details.append(QString("    Production   : %1\n").arg(IsProduction() ? "YES" : "NO"));
	details.append(QString("    Debug        : %1\n").arg(IsDebug() ? "YES" : "NO"));
	details.append(QString("\n"));
	return details;
}

//--------------------------------------------------------------------------------------------------
//	GetUser ( private )
//--------------------------------------------------------------------------------------------------
QString SystemDetails::GetUser()
{
	QString name = qgetenv("USER");
	if (name.isEmpty())
		name = qgetenv("USERNAME");
	return name;
}

//--------------------------------------------------------------------------------------------------
//	GetIsDebug ( private )
//--------------------------------------------------------------------------------------------------
bool SystemDetails::GetIsDebug()
{
#ifdef NDEBUG
	return false;
#else
	return true;
#endif
}

