#pragma once
#include <QStringList>
#include <QDataStream>

/**
@brief Override of QString that always lowercase.

Forces lowercase everywhere.
*/
class StringLC : public QString
{
public:
	StringLC(void);
	StringLC(const char* pszText);
	StringLC(const QString& other);
	virtual ~StringLC(void);

	
	StringLC& operator=(const QString& other);

	bool operator<(const StringLC& other) const;
	bool operator>(const StringLC& other) const;
	bool operator==(const StringLC& other) const;
	bool operator!=(const StringLC& other) const;
	bool operator==(const QString& other) const;
	bool operator!=(const QString& other) const;
};

QDataStream &operator>>(QDataStream &in, StringLC &str);

/**
@brief String List class for holding StringLC instances.

Handy companion class to StringLC.  
*/
class StringListLC : public QList<StringLC>
{
public:
	StringListLC();
	StringListLC(const QList<StringLC>& other);
	StringListLC& operator=(const QList<StringLC>& other);

	QStringList ToStringList() const;
};