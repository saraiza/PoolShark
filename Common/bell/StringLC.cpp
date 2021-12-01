
#include "StringLC.h"


StringLC::StringLC(void)
{
}

StringLC::StringLC(const char* pszText)
	: QString(pszText)
{
	*this = this->toLower();
}

StringLC::StringLC(const QString& other)
{
	*this = other;
}


StringLC::~StringLC(void)
{
}

StringLC& StringLC::operator=(const QString& other)
{
	QString *pThis = this;
	*pThis = other.toLower();
	return *this;
}



bool StringLC::operator<(const StringLC& other) const
{
	return (compare(other, Qt::CaseInsensitive) < 0);
}

bool StringLC::operator>(const StringLC& other) const
{
	return (compare(other, Qt::CaseInsensitive) > 0);
}

bool StringLC::operator==(const StringLC& other) const
{
	return (0 == compare(other, Qt::CaseInsensitive));
}

bool StringLC::operator!=(const StringLC& other) const
{
	return (0 != compare(other, Qt::CaseInsensitive));
}


bool StringLC::operator==(const QString& other) const
{
	return (0 == compare(other, Qt::CaseInsensitive));
}

bool StringLC::operator!=(const QString& other) const
{
	return (0 != compare(other, Qt::CaseInsensitive));
}


QDataStream &operator>>(QDataStream &in, StringLC &str)
{
	QString s;
	in >> s;
	str = s;	// The assignment along will convert to lowercase
	return in;
}


/***************************************************/


StringListLC::StringListLC()
{

}

StringListLC::StringListLC(const QList<StringLC>& other)
{
	foreach(const StringLC& s, other)
		append(s);
}

QStringList StringListLC::ToStringList() const
{
	QStringList sl;
	foreach(const StringLC& s, *this)
	{
		sl += s;
	}
	return sl;
}

StringListLC& StringListLC::operator=(const QList<StringLC>& other)
{
	// Internal down-cast
	QList<StringLC> *pThis =(QList<StringLC>*)this;
	*pThis = other;
	return *this;
}
