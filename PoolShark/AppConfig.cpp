#include "AppConfig.h"
#include "MainWindow.h"

AppConfig::AppConfig(MainWindow* pMainWindow)
{
	m_pMainWindow = pMainWindow;
}


BEGIN_SERMIG_MAP(AppConfig, 1, "AppConfig")
	SERMIG_MAP_ENTRY(1)
END_SERMIG_MAP

void AppConfig::SerializeV1(Archive& ar)
{
	if (ar.isStoring())
	{
		Qt::WindowStates ws = m_pMainWindow->windowState();
		ar << (int)ws;
		if (Qt::WindowMaximized != ws)
			ar << m_pMainWindow->saveGeometry();	// I don't get it, will have to clean this up.
		else
		{
			// Maximized, handle it differently. What screen are we on?
			ar << m_pMainWindow->geometry();
		}
		return;
	}

	Q_ASSERT(ar.isLoading());
	Qt::WindowStates ws = ar.ReadEnum<Qt::WindowStates>();

	QByteArray ba;

	if (Qt::WindowMaximized != ws)
		m_pMainWindow->restoreGeometry(ar.ReadByteArray());
	else
	{
		QRect rc;
		ar >> rc;
		m_pMainWindow->setGeometry(rc);
		m_pMainWindow->showMaximized();
	}
}
