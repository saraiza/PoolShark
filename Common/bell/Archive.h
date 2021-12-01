#pragma once
#include <type_traits>
#include <QByteArray>
#include <QTimeZone>
#include "StringLC.h"
#include "SerMig.h"
#include "DynObj.h"


class SerMig;

/**
@brief Archive is the base archive for the SerMig system

This was created to be able to polymorph the archive behavior. For years it
was a binary-only sytem. Now we have added a text option. 
*/
class Archive
{
protected:
    /// Private constructor, you must call Create.
    Archive(bool bStoring, SerMig::Options options);

public:
	static Archive* Create(QIODevice *pDevice, SerMig::Options options);

	virtual ~Archive() { }

	// Create a new archive of the same type and settings
	Archive* Clone(QIODevice *pStream) const;

	/// Read or write the archive format type tag. We 'reuse' a single Start() method
	/// at the beginning. On write, it writes the tag to identify the format for later.
	/// On read, it reads something from the stream to identify if it is the format
	/// that it supports. If it returns true, then it matches and the archive can
	/// then be used to deserialize from.
	virtual bool Start() = 0;

	virtual QDataStream::FloatingPointPrecision floatingPointPrecision() const = 0;
	virtual void setFloatingPointPrecision(QDataStream::FloatingPointPrecision newPrec) = 0;
	bool isStoring() const;
	bool isLoading() const;
	int ReadInt();
	uint ReadUInt();
	bool ReadBool();
	float ReadFloat();
	double ReadDouble();
	size_t ReadSizeT();
	template<class T> T ReadEnum() { return (T)ReadInt(); }
	QString ReadString();
	QByteArray ReadByteArray();
	QStringList ReadStringList();
    QDateTime ReadDateTime();
	QDateTime ReadDateTimeQt4();	///< For reading data created before the switch to Qt5
	QVariant ReadVariant();

	// Text-specific hints. These are ignored in a binary format
	virtual Archive& label(const QString& sLabel) { return *this; } ///< Allows you to use: ar.label("clr") << m_clr;
	virtual void Tag(const QString& sTag) {}	///< Serializers can add this to insert a 'readable' string in the output. 
	virtual void Indent() {};
	virtual void Unindent() {};

	virtual void WriteRawData(const void *pData, int iLen) = 0;
	virtual int ReadRawData(void *pData, int iLen) = 0;

	virtual bool DebugTag() = 0;	///< Read or Write a debug tag. When reading, returns true if tag is valid

	virtual bool atEnd() const = 0;


	// Low-level native types
	virtual void operator>>(qint8 &i) = 0;
	virtual void operator<<(qint8 i) = 0;
	virtual void operator>>(quint8 &i) = 0;
	virtual void operator<<(quint8 i) = 0;
	virtual void operator>>(qint16 &i) = 0;
	virtual void operator<<(qint16 i) = 0;
	virtual void operator>>(quint16 &i) = 0;
	virtual void operator<<(quint16 i) = 0;
	virtual void operator>>(qint32 &i) = 0;
	virtual void operator<<(qint32 i) = 0;
	virtual void operator>>(quint32 &i) = 0;
	virtual void operator<<(quint32 i) = 0;
	virtual void operator>>(qint64 &i) = 0;
	virtual void operator<<(qint64 i) = 0;
	virtual void operator>>(quint64 &i) = 0;
	virtual void operator<<(quint64 i) = 0;
	virtual void operator>>(bool &i) = 0;
	virtual void operator<<(bool i) = 0;
	virtual void operator>>(float &f) = 0;
	virtual void operator<<(float f) = 0;
	virtual void operator>>(double &f) = 0;
	virtual void operator<<(double f) = 0;

	// Assorted Qt classes and other common types
	virtual void operator<<(const QChar& thing) = 0;
	virtual void operator>>(QChar &thing) = 0;
	virtual void operator>>(QString &s) = 0;
	virtual void operator<<(const QString& s) = 0;
	virtual void operator>>(QByteArray &ba) = 0;
	virtual void operator<<(const QByteArray& ba) = 0;
	virtual void operator>>(QVariant &var) = 0;
	virtual void operator<<(const QVariant& var) = 0;
	virtual void operator>>(QDateTime &thing) = 0;
	virtual void operator<<(const QDateTime& thing) = 0;
	virtual void operator>>(QDate &thing) = 0;
	virtual void operator<<(const QDate& thing) = 0;
	virtual void operator>>(QTime &thing) = 0;
	virtual void operator<<(const QTime& thing) = 0;
	virtual void operator>>(QTimeZone &thing) = 0;
	virtual void operator<<(const QTimeZone& thing) = 0;
	virtual void operator<<(const QUuid& thing) = 0;
	virtual void operator>>(QUuid &thing) = 0;
	virtual void operator<<(const QPoint& thing) = 0;
	virtual void operator>>(QPoint &thing) = 0;
	virtual void operator<<(const QPointF& thing) = 0;
	virtual void operator>>(QPointF &thing) = 0;
	virtual void operator<<(const QImage& thing) = 0;
	virtual void operator>>(QImage &thing) = 0;
	virtual void operator<<(const QColor& thing) = 0;
	virtual void operator>>(QColor &thing) = 0;
	virtual void operator<<(const QTransform& thing) = 0;
	virtual void operator>>(QTransform &thing) = 0;
	virtual void operator<<(const QRect& thing) = 0;
	virtual void operator>>(QRect &thing) = 0;
	virtual void operator<<(const QRectF& thing) = 0;
	virtual void operator>>(QRectF &thing) = 0;


	// Built-in to Archive, they forward to the above methods internally
	void operator>>(std::nullptr_t &ptr) { ptr = nullptr; }
	void operator<<(std::nullptr_t) {  }

	template<class T> T Read()		///< Generic type read
	{
		static_assert(std::is_default_constructible<T>::value, "Type `T` must be default constructible");
		T dv;
		*this >> dv;
		return dv;
	};
	/// Useful method for reading an enum value in one line.
	template<class TEnum> void toEnum(TEnum& enumVar)
	{
		int iVal;
		*this >> iVal;
		enumVar = (TEnum)iVal;
	}


	void operator<<(StringLC slc)
	{
		QString *pStrLC = &slc;
		*this << *pStrLC;
	}

	void operator>>(StringLC &slc)
	{
		QString s;
		*this >> s;
		slc = s;
	}


	// Not for you to call, used internally by the macros.
	void _DebugTagEnd(SerMig *pSerMig);

	// Use this when you are not sure of the type (such as polymorphism being used)
	//  This handles duplicate objects correctly (not quite yet!). You must register your
	//  custom class with Q_DECLARE_METATYPE/qRegisterMetaType! See example in the above
	//  comments. These methods properly handle duplicate object instances.
	void WriteDObj(SerMig *pObj);
	SerMig* ReadDObj();

	// Use these when you know the type and do not need polymorphism.  These methods
	// properly handle duplicate object instances.
	template<class TClass> void WriteObj(TClass *pObj)
	{
		WriteDObj(pObj);
	}

	template<class TClass>
	TClass* ReadObj()
	{
		// Read the object ID from the stream
		int id;
		*this >> id;

		// Special case for NULL pointer
		if(0 == id)
			return NULL;

		// Have we seen this object before?
		QMap<int, SerMig*>::const_iterator iter;
		iter = m_objMapR.find(id);
		if(m_objMapR.end() != iter)
		{
			// Yes, so it is a duplicate pointer.
			// Just return the object
			return (TClass*)iter.value();
		}

		// No, we have not seen this object before. Build it!

		// Grab the class name
		QString className;
		*this >> className;

		// Create an instance of the class (by name)
		TClass *pNewObj = DynObj::Create<TClass>(className);
		if(NULL == pNewObj)
			throw SerMig::ArchiveException(QString("Could not create instance of type '%1'").arg(className));

		// Save this object in our map. Must be done before we call Serialize
		AddObjR(id, pNewObj);

		// Deserialize the data into our new object
		pNewObj->Serialize(*this);

		return pNewObj;
	}

	int AddObjW(SerMig *pObj);
	void AddObjR(int id, SerMig *pObj);


protected:
	void InitDebugTags();	///< Must be called only after m_bStoring has been set
	SerMig::Options m_options;	///< Passed-in at creation

private:
	bool m_bStoring;

	// Object pointer serialization
	int m_iNextID;
	bool m_bUsingDebugTags;
	QMap<SerMig*, int> m_objMapW;	// Used for writing
	QMap<int, SerMig*> m_objMapR;	// Used for reading
};




// Support for Qt containers. Copied from QDataStream (private).
namespace SerMigPrivate
{
	template <typename Container>
	void ReadArrayBasedContainer(Archive &ar, Container &c)
	{
		c.clear();
		quint32 iCount;
		ar.label("count") >> iCount;
		c.reserve(iCount);
		for (quint32 i = 0; i < iCount; ++i) {
			typename Container::value_type t;
			ar >> t;
			c.append(t);
		}
	}

	template <typename Container>
	void ReadListBasedContainer(Archive &ar, Container &c)
	{
		c.clear();
		quint32 iCount;
		ar.label("count") >> iCount;
		for (quint32 i = 0; i < iCount; ++i) {
			typename Container::value_type t;
			ar >> t;
			c << t;
		}
	}

	template <typename Container>
	void ReadAssociativeContainer(Archive &ar, Container &c)
	{
		c.clear();
		quint32 iCount;
		ar.label("count") >> iCount;
		for (quint32 i = 0; i < iCount; ++i) {
			typename Container::key_type k;
			typename Container::mapped_type t;
			ar >> k;
			ar >> t;
			c.insertMulti(k, t);
		}
	}

	template <typename Container>
	void WriteSequentialContainer(Archive &ar, const Container &c)
	{
		ar.label("count") << quint32(c.size());
		ar.Indent();
		for (const typename Container::value_type &t : c)
			ar << t;
		ar.Unindent();
	}

	template <typename Container>
	void WriteAssociativeContainer(Archive &ar, const Container &c)
	{
		ar.label("count") << quint32(c.size());
		ar.Indent();

		// Always sort maps as we store them to make the data deterministic
		auto keys = c.keys();
		std::sort(keys.begin(), keys.end());
		for (auto key : keys)
		{
			ar << key;
			ar << c.value(key);
		}
		ar.Unindent();
	}

} // namespace SerMigPrivate


template <typename T>
inline void operator>>(Archive &ar, QList<T> &l)
{
	SerMigPrivate::ReadArrayBasedContainer(ar, l);
}

template <typename T>
inline void operator<<(Archive &ar, const QList<T> &l)
{
	SerMigPrivate::WriteSequentialContainer(ar, l);
}





template <typename T>
inline void operator>>(Archive &ar, QSet<T> &set)
{
	QList<T> list;
	ar >> list;
	set = list.toSet();
}

template <typename T>
inline void operator<<(Archive &ar, const QSet<T> &set)
{
	QList<T> list = set.values();
	std::sort(list.begin(), list.end());
	ar << list;
}



template <class Key, class T>
inline void operator>>(Archive &ar, QHash<Key, T> &hash)
{
	SerMigPrivate::ReadAssociativeContainer(ar, hash);
}

template <class Key, class T>
inline void operator<<(Archive &ar, const QHash<Key, T> &hash)
{
	SerMigPrivate::WriteAssociativeContainer(ar, hash);
}



template <class Key, class T>
inline void operator>>(Archive &ar, QMap<Key, T> &map)
{
	SerMigPrivate::ReadAssociativeContainer(ar, map);
}

template <class Key, class T>
inline void operator<<(Archive &ar, const QMap<Key, T> &map)
{
	SerMigPrivate::WriteAssociativeContainer(ar, map);
}



template <typename Enum>
inline void operator<<(Archive& ar, QFlags<Enum> e)
{
	ar << e.i;
}

template <typename Enum>
inline void operator>>(Archive &ar, QFlags<Enum> &e)
{
	ar >> e.i;
}



template <class T1, class T2>
inline void operator>>(Archive& s, QPair<T1, T2>& p)
{
	s >> p.first;
	s >> p.second;
}

template <class T1, class T2>
inline void operator<<(Archive& s, const QPair<T1, T2>& p)
{
	s << p.first;
	s << p.second;
}






