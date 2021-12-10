#pragma once
#include <SerMig.h>

class MainWindow;


class AppConfig : public SerMig
{
public:
	DECLARE_SERMIG;
	AppConfig(MainWindow* pMainWindow);

private:
	MainWindow* m_pMainWindow = nullptr;
	void SerializeV1(Archive& ar);
};

