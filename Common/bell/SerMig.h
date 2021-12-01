#pragma once
#include <type_traits>

/**
@class SerMig
SerMig - Serialization/Migration

The full documentation can be found below, but here is a quick usage example. To make a class
serializable, it must inherit from `SerMig` and have a few macros thrown in as shown:

~~~~~~~~~~~~~~~~~~~~~{.c}

//// MyClass.h
class MyClass : public SerMig
{
public:
	MyClass();
	MyClass(const MyClass& other);

	DECLARE_SERMIG;		// << You need this

private:
	void SerializeV2_289(Archive& ar);	// << Your functions must be named SerializeVx_yy!
	void SerializeV1_3512(Archive& ar);

	QString m_companyName;
	int m_iEmployees;	
	MySubClass m_subThing;
};

Q_DECLARE_METATYPE(MyClass)	// << You need this for dynamic object creation
SERMIG_ARCHIVERS(MyClass)	// << Adds some handy << and >> operator overloads for your new class

//// MyClass.cpp

//// The serializtion map
BEGIN_SERMIG_MAP(MyClass, 289, "MyClass") // << the string is how you 'register' the class for dynamic
  SERMIG_MAP_ENTRY2(2, 289)				//      creation. This allows you to change the actual class name later if you wish.
  SERMIG_MAP_ENTRY2(1, 3512)
END_SERMIG_MAP


void MyClass::SerializeV2_289(Archive& ar)
{
	if(ar.isStoring())					// This writing code might get deleted
	{									// when you add SerializeV3 later on,
		// Write						// so keeping this structure makes that
		ar << m_companyName;			// quicker to do.
		ar.label("emp") << m_iEmployees;// label() is used by the text archiver
		ar << m_subThing;		// 
		return;
	}

	// Read code	
	ar >> m_companyName;
	ar.label("emp") >> m_iEmployees;
	m_subThing.Serialize(ar);	// Assume this was added in V2
}


void MyClass::SerializeV1_3512(Archive& ar)
{
	////if(ar.isStoring())			// Only the reading code is needed
	////{								// for this older version
	////	// Write
	////	ar << m_companyName;
	////	ar.label("emp") << m_iEmployees;
	////	return;
	////}
	Q_ASSERT(ar.isLoading());
	
	// Read	
	ar >> m_companyName;
	ar.label("emp") >> m_iEmployees;
}


//// usage example, writing to a byte stream
MyClass co;
co.m_companyName = QString("Aperture Science");
co.m_iEmployees = 9;

QByteArray blob;
Archive arW(&blob, QIODevice::WriteOnly);
co.Serialize(arW);

//// Deserializing from that same blob
Archive arR(&blob, QIODevice::ReadOnly);
CCompany co2;
co2.Serialize(arR);

~~~~~~~~~~~~~~~~~~~~~


Overview
--------
The SerMig class provides the serialization system for our software.
Serialization means converting an object or object tree to a byte  
stream and later recreating it in the same state from that byte stream. 
The two most important design goals of this class is to provide a simple
to use way to persist an object to a stream/file and to be able to open
every previous version of every file we have ever generated.

I've written a lot of documentation here, but the implementation is small 
and simple.

### Features

+ Almost any type can be serialized
+ Per-object versioning
+ Dynamic object creation by type name
+ Handles duplicate object pointers in an object tree
+ Plays well with branching (use SERMIG_MAP_ENTRY2 macro instead of SERMIG_MAP_ENTRY)
+ Abstracts the stream format using the Archive base class

### Rationale

Dealing with file formats is difficult.  The code to parse a data file
can get very complicated because it must successfully parse every previous
version of it that was ever written.  

What usually happens in a company is that R&D mgmt will tell you,
"Don't worry about it.  We will just 'release' a new main file format
version as part of our release process, and test it then.  No, we won't
have to bother supporting the unreleased intermediate versions."  Sounds 
nice, but that's complete bullshit/wishfull thinking.  

During the R&D lifetime of a product, it will be used and tested in a 
large variety of situations, and there will always be some data created by 
your software that is important to someone.  At some point, you will be asked  
to migrate some previous version of a file format for some important customer 
(or future customer). You will have to do that by building a previous version 
of the product, and then trying to hack a way to convert it to something the 
current build can read or convert.

When release time comes, you must write explicit format version migration
code to convert previous versions into the new versions.  This code is always
the last code you write.  You can't write it too soon, the new format is not
stable until release time! So...big new feature, only tested late in the dev
process? Sounds scary? It is.

SirMig was designed to force version migration to be an integral
part of the deveopment process.  Here are the requirements it was designed
around:

+ Easy to use
+ The reading and writing code are always written at the same time
+ The reading and writing code are co-located
+ Migration code is written when the format code is changed (by the author of the change)
+ Breaking the reading/migration code causes a compile error, not a runtime error
+ Separate reading functions for every previous version (not one for all)

When a developer is forced to add the migration code they change the version format, 
they are doing it at the moment when they are most knowledgable about
the details of change.  Doing it later means a lot more work because it becomes a 
research project as well. Also, the code will get tested just by regular usage.

Forcing the developer to handle the version migration everytime he changes the
format sounds onerous, but it's not.  The developer still has the option of
not bothering and creating an orphaned version, that's his/her call. But keeping
the process simple and light makes it more likely that the devloper will just
follow the rules and write migration code when things change.


### History

I originally implemented this back in 2002 . It was based on MFC's 
CArchive-based serialization. MFC's serialization was a wonderful 
design, but file format versioning always proved to be a problem.  
So I came up with CSerializeMigrator to provide per-object versioning.  

This is the 3rd rewrite of that original system.  It is now natively 
based on Qt and I added dynamic object serialization (Qt-style which
means that you can serialize pointers to classes. It also handles 
duplicate objects correctly, you can now serialize a complex tree of objects.

I later learned that at the same time I originally implemented serialization,
Robert Ramey was writing [boost serialization] (http://www.boost.org/libs/serialization/).  
He came to the same conclusions I did about what the acutal problems are when
persisting data to a file, but his boost implementation was more full-featured
(and more complicated). He also made up many more design requirements. I kept 
mine simple.

### Multiple Formats

The system uses separate Archive* classes to implement the actual file rendering
into a specific format.  Currently binary and text are implmented.  The Archive
class is polymorphed to provide this.  There are a few methods on Archive to help
support text format:

+ Tag():  Inject any text you want when writing, r/w must match
+ Indent()/Unindent(): Standard block indent formatting
+ label(): Puts a nice string on the line of the variable, ar.label("pet name") << m_sPetName;

### Usage

<See above for some simple examples>

Usually, you have an object tree that represents your data or 'document',
and you want to serialize the entire tree.  To do so, you would derive
each serializable object class from SerMig and add the required macros 
to it.

The serializtion map macros in the cpp file is what hooks a version 
number to a serialization function for a class.  You will start with
just one serialization function, implement it like this (duplicate snippet):

~~~~~~~~~~~~~~~~~~~~~{.c}
void MyClass::SerializeV1(Archive& ar)
{
	if(ar.isStoring())			
	{						
		// Write			
		ar << m_companyName;
		ar << m_iEmployees;
		ar << m_subThing;		// The SERMIG_ARCHIVERS macro lets you use this notation instead of m_subThing.Serialize(ar)
		return;
	}

	// Read 	
	ar >> m_companyName;
	ar >> m_iEmployees;
	ar >> m_subThing;
}
~~~~~~~~~~~~~~~~~~~~~

Note that you implement both writing and reading at the same time. This
structure allows you to easily keep the symmetry.  It must be symmetric,
these are NOT name/value pairs!  The order of the member variable reading
MUST match the order of the writing.

#### Serializer Naming and Branching

After using SerMig for a while, we realized that we often had problems when a branch
was created that lived for a while.  If two developers add serializers for the same
class in different branches, there will always be a collision. If output files are
created by these different versions, you will never be able to read both versions
from your software because you have no way to identify which code to use for the
conflicted object version. 

Because of this, you have the option to use a non-sequential object version number.  
This should be a randomized and hopefully unique number. It only needs to be unique
within the class.

Notice that we did not just switch to using randomized version numbers, we still have
the sequential version numbers. This is because it is very useful for developers to
know the chronological order of the serializers.  When branching happens, we may
have duplicate sequential numbers such as SerializerV5_123() and SerializerV5_1954(),
but that's ok.

#### Simple Inheritance

If you have inherited a class from another serializable class, your derived
class should call the base class's Serialize first, and then sererialize only
the added members.  Something like this:

~~~~~~~~~~~~~~~~~~~~~{.c}
void MyDerivedClass::SerializeV1(Archive& ar)
{
	MyBaseClass::_SerializeMap(ar);

	if(ar.isStoring())			
	{						
		//// Write			
		ar << m_companyName;
		ar << m_iEmployees;
		m_subThing.Serialize(ar);
		return;
	}

	//// Read 	
	ar >> m_companyName;
	ar >> m_iEmployees;
	m_subThing.Serialize(ar);
}
~~~~~~~~~~~~~~~~~~~~~


You must add the scope qualifier to call out the base class, otherwise you
will have an infinite call chain and cause a stack overflow.

### Migration

Now we will walk through a few examples of different types of future 
modifications to the class, and how you would deal with migration for those
cases.

#### Migration: Add Member to a class

Suppose you need to modify the class by adding an address string. You will
need to add a new serializer method and the associated entry in the map.

~~~~~~~~~~~~~~~~~~~~~{.c}
class MyClass : public SerMig
{
	...
private:
	void SerializeV2_156(Archive& ar);	// << Added
	void SerializeV1_3256(Archive& ar);

	...
	QString m_companyName;
	int m_iEmployees;	
	QString m_addr;				// << Added
};
~~~~~~~~~~~~~~~~~~~~~

Modify the SERMIG map like this:
~~~~~~~~~~~~~~~~~~~~~{.c}

	BEGIN_SERMIG_MAP(MyClass, 156, "MyClass") // << Change current version to 156
	  SERMIG_MAP_ENTRY2(2, 156)					// Add the new version entry
	  SERMIG_MAP_ENTRY2(1, 3256)
	END_SERMIG_MAP
~~~~~~~~~~~~~~~~~~~~~

And implement the new method. You start with a copy & paste from the existing
serialize method, then make your modification:

~~~~~~~~~~~~~~~~~~~~~{.c}
void MyClass::SerializeV2_156(Archive& ar)
{
	if(ar.isStoring())			
	{							
		//// Write			
		ar << m_companyName;
		ar << m_iEmployees;		
		ar << m_addr;			// 
		m_subThing.Serialize(ar);
		return;
	}

	//// Read code	
	ar >> m_companyName;
	ar >> m_iEmployees;
	m_subThing.Serialize(ar);	// Assume this was added in V2
}
~~~~~~~~~~~~~~~~~~~~~
	Note that in this case, you don't have to do anything to SerializeV1.
	The only thing you need to be wary of is that the m_addr for a version
	1 object will have it's default value, empty.  

	Since the writing code for the older versions should never be called
	again, you can delete it from SerializeV1:

~~~~~~~~~~~~~~~~~~~~~{.c}
void MyClass::SerializeV1_3256(Archive& ar)
{				
	Q_ASSERT(!ar.isLoading());	// << Leave an assert just for sanity!

	// Read code	
	ar >> m_companyName;
	ar >> m_iEmployees;
}
~~~~~~~~~~~~~~~~~~~~~

#### Migration: Deleting a member from a class
	
Now we decide to remove the m_iEmployees member because it was ambiguous
and some idiot just put it in as a 'placeholder'. So we delete it from the 
class and add a new serializer. 

When we build, the compiler gives us compile errors in the previous serializer!
This is good because we can't defer fixing the compile error:

~~~~~~~~~~~~~~~~~~~~~{.c}
void MyClass::SerializeV2_156(Archive& ar)
{
	Q_ASSERT(ar.isLoading());

	//// Read 	
	ar >> m_companyName;
	ar >> m_iEmployees;		<< ERROR! This member is gone!
	m_subThing.Serialize(ar);
}
~~~~~~~~~~~~~~~~~~~~~

In this case, we know that we can fix it by just ignoring the data. WE STILL
NEED TO READ IT! So we modify like this:

~~~~~~~~~~~~~~~~~~~~~{.c}
void MyClass::SerializeV2_156(Archive& ar)
{
	Q_ASSERT(ar.isLoading());

	//// Read 	
	ar >> m_companyName;
	int iEmployees_obsolete;	// We never used this
	ar >> iEmployees_obsolete;
	m_subThing.Serialize(ar);
}
~~~~~~~~~~~~~~~~~~~~~

A note about placeholders: **Don't create them!**  Using a placeholder for a member variable
that you _think_ you will need later breaks the concepts and principals of serialization.  You 
are creating a future situation where you will have no idea when the data for the placeholder
member became meaningful (ie, when it stopped being a placeholder!).  Placeholders in serialization
will cause you pain, so __don't do it__!.

#### Migration: Changing the meaning of, or renaming member
	
Suppose we had a boolean member m_bOff and we decided that it should be
m_bOn instead. Follow the same process. Add the new serialization method.
in the old serialize methods, you will need to migrate the old value like this:

	original:
		ar >> m_bOff

	migrated:
		bool bOff;
		ar >> bOff;
		m_bOn = !bOff;	

#### Polymorphic Serialization

If you need to serialize an object like IAnimal (abstract), use the WriteDObj/ReadDObj methods.
Your base class should derive from SerMig.

A SerMig class may need to polymorphically persist a sub-object that is **not** a SerMig class. 
This is possible by deriving the root class from DynObj and following the instructions in DynObj.h.

#### QObject Serialization

If you are going to serialize a QObject, then you must provide a default constructor
__and__ a default copy constructor.  Qt intentially disables automatic copy constructor
generation (with the Q_DISABLE_COPY macro) because it does not usually make sense to
copy a QObject. 

If you provide a copy constructor, be sure to either implement it correctly or add a
Q_ASSERT(false) to it. Also, make sure you use the VERIFY__SIZE macro in your
constructor implementation to make sure you don't forget about the copy constructor 
when you add members to the class.

So far, you may not use dynamic serialization of QObject classes. Be sure to use the
templated versions of ReadObj. For some reason, QObject-derived classes do not construct
correctly when assigned to a SerMig* instead of the actual class pointer. I'm not sure
what's going on, perhaps you will fix it for me. Thanks in advance, Steve.

#### Inheritance

If you derive a class from a serialized class, the serializers for the two classes remain
independent.  For example,  if Sneaker derives from Shoe, then each class maintains a separate
serialization map and separate serialize methods.  Sneaker::SerializeVx must first invoke
the Shoe serializer before doing the sneaker-specific serialization.

But how is this done? You can't call Shoe::Serialize(ar) because the C++ virtual chain 
will take you right back to Sneaker::Serialize(ar).  You must directly invoke the Shoe 
serialization map by name like this:
~~~~~~~~~~~~~~~~~~~~~{.c}
void Sneaker::SerializeV2(Archive& ar)
{
	Shoe::_SerializeMap(ar);

	if(ar.isStoring())
	{
		ar << m_iLaceHoles;
		return;
	}

	// Read code	
	ar >> m_iLaceHoles;
}
~~~~~~~~~~~~~~~~~~~~~

I know, it's ugly. But I don't have a better solution for it.


#### Debug Tags

When using serialization, the order of the r/w serialization steps must be kept symmetric.  You must
read the exact same number of bytes that you wrote.  If you don't, bad things may happen and it can be
hard to find the cause.  

To help you check your code, you can turn on the Debug Tag feature by uncommenting the SERMIG_DEBUG_TAGS
#define.  Doing so will cause a tag to be written to the archive after every object.  When reading, the
tag will be read in, checked, and an exception thrown if it is not right.  Use this feature to find 
errors in your SerializeVx methods.

There is a bool that is written to the archive that indicates if the debug tags are in the stream. Because
of this bool, it is save to mix and match usage of the Debug Tags feature.  In other words, data files 
generated with this feature enabled can be read in the future by code with the feature turned off.


### Internals:
	
*/
#include <QDataStream>
#include "Exception.h"
#include <QMap>
#include "Macros.h"
#include <QFile>
#include "Defs.h"

class Progress;
class Archive;



/// If you think you are screwing up the symmetry of serialization somewhere, uncomment
/// the SERMIG_DEBUG_TAGS #define and run the tests.  This will add tags before and after each
/// serialized object in the stream and assert on errors. YOU MAY LEAVE THIS IN IF YOU WANT!
/// Debug Tags left in a file can still be readable after the SERMIG_DEBUG_TAGS #define is
/// turned off.
#define SERMIG_DEBUG_TAGS


// This typedef determines the data format used for the
// object version number.  (Changing this will invalidate 
// all of your file formats!)
typedef unsigned short FORMATVER;


class SerMig
{
	friend class Archive;
    Q_GADGET
public:
	class ArchiveException : public Exception
	{
	public:
		ArchiveException(QString msg) : Exception(msg) {}

		ArchiveException(QString msg, QString className, int fmtVer) :
			Exception(QString("%1 (class %1, version %2)").arg(className, QString::number(fmtVer))) { }
	};

	// This is thrown if a newer object format is detected
	class UnknownFormatException : public ArchiveException
	{
	public:
		UnknownFormatException(const QString& className, int fmtVer) : ArchiveException("Unknown object format version number",className, fmtVer) { }
		UnknownFormatException() : ArchiveException(QString("Unknown object version number")) { }
        virtual ~UnknownFormatException() throw() { }
	};

	// This is thrown if the tags are not right. This means you screwed up the symmetry of the serializer.
	class SerMigTagException : public ArchiveException
	{
	public:
		SerMigTagException(const char* pszMsg) : ArchiveException(QString("Bad SerializeVx symmetry-%1").arg(pszMsg)) { }
		SerMigTagException() : ArchiveException(QString("Bad SerializeVx symmetry")) { }
        virtual ~SerMigTagException() throw() { }
	};

	class ChecksumException : public ArchiveException
	{
	public:
		ChecksumException(const QString& sErr) : ArchiveException(sErr) { }
		virtual ~ChecksumException() throw() { }
	};

public:
	SerMig(void);
	virtual ~SerMig(void);

	enum Option
	{
		OPT_None = 0,
		OPT_DebugTags = 1,
        OPT_Checksum = 2,				///< Checksum the data (implies binary format)
        OPT_ChecksumVerifyOnWrite = 4,  ///< Verify the checksum immediately after writing.
		OPT_Text = 8,					///< Always select the text implementation
		OPT_Binary = 9,					///< Always select the binary implementation
		OPT_Naked = 15,					///< No special things encoded. No Start()
		OPT_EnsureFlushed = 16,		    ///< Uses unbuffered write and (on windows) asks the file system to flush any cached writes
	};
	Q_DECLARE_FLAGS(Options, Option)

	/// Serialize the object into or out of an archive. 
	void Serialize(Archive& ar); // Derived classes just forward to _Serialize() which forward to _SerializeMap()

	/// Special implementation that uses the old legacy method. We 
	/// want to eventually remove these, we probably never will to
	/// keep compatability with old formats. :(
	void Serialize_LEGACY(Archive& ar);

	/// You can query the current serialization write level. (this is filled in by the macro)
	virtual int GetWriteFmtVer() = 0;

	// Derived classes override this if they are participating in dynamic object creation
	virtual QString className() = 0;

	/// Tested for the TestSerializers test suite in our Test project. You should implement
	/// both to get a better test for your derived class. Don't forget to add your new class 
	/// to the test project.
	virtual void testInit(const QString& /*sDataDir*/) {}
	virtual bool testCompare(SerMig *pOther) const { return true; }

	// Some utility functions
	QByteArray toBlob(Options options = OPT_None) const;
    void fromBlob(const QByteArray& blob, Options options = OPT_None);
	void toFile(QIODevice *pDevice, Options options = OPT_None) const;
	void toFile(const QString& filename, Options options = OPT_None) const;
	void fromFile(QIODevice *pDevice, Options options = OPT_None);
	void fromFile(const QString& filename, Options options = OPT_None);

	/// Special filename handling to make the save/load atomic.
	void toFileAtomic(const QString& filename, Options options = OPT_None) const;
	void fromFileAtomic(const QString& filename, Options options = OPT_None);

	static QString VerifyChecksum(const QString& filename, Progress& prog);

	QString toPctEncodedString() const;
	void fromPctEncodedString(const QString& strTxt);
	void toPctEncodedTxtFile(const QString& filename, Options options = OPT_None) const;
	void fromPctEncodedTxtFile(const QString& filename);

protected:
	// Derived classes must implement this by using the macros
	virtual void _Serialize(Archive& ar) = 0;
	void _Serialize_Begin(Archive& ar, FORMATVER fmtVerWrite);	///< Not overridden for easy debug
	void _Serialize_End(Archive& ar);							///< Not overridden for easy debug

	FORMATVER m_formatVer;

    /// Rewind to start, read all data, verify the checksum footer.
	/// Throws no exceptions, just returns an error string if there
	/// was a problem. The caller decides what to do with it.
    static QString _VerifyChecksum(QIODevice *pDevice, Progress& prog);

public:
	/// Support methods for the archiver macros below
	static void LegacyWR(Archive& ar, const SerMig *pObj);
	static void LegacyRD(Archive& ar, SerMig *pObj);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SerMig::Options)

// Allowing SerMig objects to be used by QDataStreams. 
// SerMig does not need this! But the QtRpc system is
// still around and appears to need this.
inline QDataStream& operator<<(QDataStream& s, const SerMig& l)
{
	QByteArray blob = l.toBlob(SerMig::OPT_None);
	s << blob;
	return s;
}

inline QDataStream& operator>>(QDataStream& s, SerMig& l)
{
	QByteArray blob;
	s >> blob;
	l.fromBlob(blob);
	return s;
}



// The migration map macros


// Put this in the header file for your SerMig class.
// The serializer map/table will provide implementations 
// for these methods.
#define DECLARE_SERMIG \
	virtual QString className() override; \
	virtual void _Serialize(Archive& ar) override; \
	void _SerializeMap(Archive& ar); \
	void _Serialize_Route(Archive& ar); \
	virtual int GetWriteFmtVer() override 


//This is a special DynoObj register function for serializers
//that also registers the QDataStream operators with Qt's meta
//object system. This is required because not all DynoObjs have
//data stream operators.
#define DYNO_SERMIG_REGISTER(TClass, regName) \
void _DynObjRegSerMig_##TClass() \
{ \
    qRegisterMetaType<TClass>(regName); \
} \
DynObj::DynObjMetaRegistrar static gs_registrarSerMig_##TClass((void*)&_DynObjRegSerMig_##TClass);	\
\
QString TClass::className() { \
    return QString(regName); \
}



// Start a map.  Dynamic class registration code is also created from this macro.
// To make this understandable, all macro params start with _. Otherwise, it can
// get confusing between real parameters and macro arguments.
#define BEGIN_SERMIG_MAP(_TClass, _fmtVerWrite, _regName) \
QString _TClass::className() \
{ \
	return #_TClass; \
} \
\
int _TClass::GetWriteFmtVer() \
{ \
	FORMATVER format_version = _fmtVerWrite; \
	return (int)format_version; \
} \
\
void _TClass::_Serialize(Archive& ar) \
{ \
	_SerializeMap(ar); \
} \
\
void _TClass::_SerializeMap(Archive& ar) \
{ \
	_Serialize_Begin(ar, _fmtVerWrite); \
	_Serialize_Route(ar); \
	_Serialize_End(ar); \
} \
\
void _TClass::_Serialize_Route(Archive& ar) \
{ \
	FORMATVER fmtVer = ar.isStoring() ? _fmtVerWrite : m_formatVer; \
	switch (fmtVer) \
	{ 




/// This macro adds an entry to the map's switch statement which redirects to the actual serialize method
#define SERMIG_MAP_ENTRY(format_version)	\
		case format_version: \
		SerializeV##format_version(ar); \
			break; 


/// Alternate version decorates the function name further to this format: SerializeV2_23513()
/// We keep the sequence number only so that us developers can identify the chronological order
/// of the serializers
#define SERMIG_MAP_ENTRY2(verA, verB) \
		case verB: \
		SerializeV##verA##_##verB(ar); \
			break; 


#define END_SERMIG_MAP \
	case 0: \
		throw ArchiveException("No More Data to Deserialize", className(), fmtVer); \
    default: \
        throw UnknownFormatException(className(), fmtVer); \
    } \
}


/// Use this macro after your .h class declaration to create handy
/// operator mapping to your class.  This allows you to do this in
/// a serializer:  ar << myObj;
///
/// At some point, this macro should just be replaced by a template
/// function that matches everything. Before we can do that, we must
/// fix the problem mentioned below.
#define SERMIG_ARCHIVERS(TClass) \
static Archive& operator>>(Archive& ar, TClass& obj) \
{ \
	obj.Serialize(ar); \
	return ar; \
} \
static Archive& operator<<(Archive& ar, const TClass& obj)  \
{ \
	TClass *pObj = (TClass*)&obj; \
	pObj->Serialize(ar); \
	return ar; \
} 


#include "Archive.h"



/// @brief A class to modify the QDataStream's handling of floats
///
/// This is a super handy class to change the precision of floats inside your serialization methods
/// without affecting anything outside that particular object.  Since QDataStream holds floats
/// as doubles by default, enforcing precision to single can cut your data rate in HALF, 50%, 1/2
/// i.e.: A LOT!!!
/// Usage:  In your SerializeVx(Archive& ar) method, add this:
///         FloatPrecisionDefender pe(&ar, QDataStream::SinglePrecision);
class FloatPrecisionDefender
{
public:
	FloatPrecisionDefender(Archive* s, QDataStream::FloatingPointPrecision newPrec);
	~FloatPrecisionDefender();

private:
	Archive* m_stream = nullptr;
	QDataStream::FloatingPointPrecision m_oldPrec;
};


