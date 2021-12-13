#include <QApplication>
#include <QCursor>

template<Qt::CursorShape CS>
struct Cursor
{
	inline Cursor()  { QApplication::setOverrideCursor(CS);	  }
	inline ~Cursor() { QApplication::restoreOverrideCursor(); }
};

// Select one of these and create it on the stack

using WaitCursor = Cursor<Qt::WaitCursor>;
using BusyCursor = Cursor<Qt::BusyCursor>;
using ArrowCursor = Cursor<Qt::ArrowCursor>;
