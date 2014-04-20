#include "qcursor.h"
#include "qcursor_p.h"

#include <Application.h>
#include <Cursor.h>

#include <stdio.h>

int	haiku_global_mouse_x = 0;
int	haiku_global_mouse_y = 0;

BCursor *HaikuCursorCache[32]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
							   NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

int	CursorMapList[]={	Qt::ArrowCursor,		B_CURSOR_ID_SYSTEM_DEFAULT,
						Qt::UpArrowCursor,		B_CURSOR_ID_RESIZE_NORTH,
						Qt::CrossCursor,		B_CURSOR_ID_CROSS_HAIR,
						Qt::WaitCursor,			B_CURSOR_ID_PROGRESS,
						Qt::IBeamCursor,		B_CURSOR_ID_I_BEAM,
						Qt::SizeVerCursor,		B_CURSOR_ID_RESIZE_NORTH_SOUTH,
						Qt::SizeHorCursor,		B_CURSOR_ID_RESIZE_EAST_WEST,
						Qt::SizeBDiagCursor,	B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST,
						Qt::SizeFDiagCursor,	B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST,
						Qt::SizeAllCursor,		B_CURSOR_ID_MOVE,
						Qt::BlankCursor,		B_CURSOR_ID_NO_CURSOR,
						Qt::SplitVCursor,		B_CURSOR_ID_RESIZE_NORTH_SOUTH,
						Qt::SplitHCursor,		B_CURSOR_ID_RESIZE_EAST_WEST,
						Qt::PointingHandCursor,	B_CURSOR_ID_FOLLOW_LINK,
						Qt::ForbiddenCursor,	B_CURSOR_ID_NOT_ALLOWED,
						Qt::OpenHandCursor,		B_CURSOR_ID_GRAB,
						Qt::ClosedHandCursor,	B_CURSOR_ID_GRABBING,
						Qt::WhatsThisCursor,	B_CURSOR_ID_HELP,
						Qt::BusyCursor,			B_CURSOR_ID_PROGRESS
};


extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp

QPoint QCursor::pos()
{
	return QPoint(haiku_global_mouse_x,haiku_global_mouse_y);
}

void QCursor::setPos(int x, int y)
{
//	haiku_global_mouse_x = x;
//	haiku_global_mouse_y = y;
}

BCursor *QCursor::handle() const
{
	int i;
	
    if (!QCursorData::initialized)
        QCursorData::initialize();
	
	for(i=0;i<sizeof(CursorMapList)/sizeof(CursorMapList[0]);i+=2) {
		if(CursorMapList[i]==d->cshape) {
			if(HaikuCursorCache[CursorMapList[i+1]])
				return HaikuCursorCache[CursorMapList[i+1]];
		}
	}
				
	return (BCursor*)B_CURSOR_SYSTEM_DEFAULT;
}

QCursorData::QCursorData(Qt::CursorShape s)
    : cshape(s), bm(0), bmm(0), hx(0), hy(0)
{
	/* TODO: code is prone to errors. Maybe use a static QMap instead? */
	int i;
	for(i=0;i<sizeof(CursorMapList)/sizeof(CursorMapList[0]);i+=2) {
		if(CursorMapList[i]==s) {
			if(!HaikuCursorCache[CursorMapList[i+1]])
				HaikuCursorCache[CursorMapList[i+1]] = new BCursor((BCursorID)CursorMapList[i+1]);					
		}
	}
	
    ref = 1;
}

QCursorData::~QCursorData()
{ }


QCursorData *QCursorData::setBitmap(const QBitmap &bmp, const QBitmap &mask,  int x, int y)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();

    QCursorData *c = qt_cursorTable[0];
    c->ref.ref();
    return c;
}
