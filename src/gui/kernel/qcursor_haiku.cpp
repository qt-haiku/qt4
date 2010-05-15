#include "qcursor.h"
#include "qcursor_p.h"

#include <Application.h>
#include <Cursor.h>

#include <stdio.h>

int	haiku_global_mouse_x = 0;
int	haiku_global_mouse_y = 0;

BCursor *HaikuCursorCache[32]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
							   NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

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

#ifndef QT_NO_CURSOR
BCursor *QCursor::handle() const
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
	if(d->cshape==Qt::IBeamCursor)
		return HaikuCursorCache[B_CURSOR_ID_I_BEAM];
	if(d->cshape==Qt::PointingHandCursor)
		return HaikuCursorCache[B_CURSOR_ID_FOLLOW_LINK];
	if(d->cshape==Qt::CrossCursor)
		return HaikuCursorCache[B_CURSOR_ID_CROSS_HAIR];
	if(d->cshape==Qt::SplitHCursor)
		return HaikuCursorCache[B_CURSOR_ID_RESIZE_EAST_WEST];
	if(d->cshape==Qt::SplitVCursor)
		return HaikuCursorCache[B_CURSOR_ID_RESIZE_NORTH_SOUTH];
	if(d->cshape==Qt::BlankCursor)
		return HaikuCursorCache[B_CURSOR_ID_NO_CURSOR];
	if(d->cshape==Qt::WhatsThisCursor)
		return HaikuCursorCache[B_CURSOR_ID_HELP];
		
	return (BCursor*)B_CURSOR_SYSTEM_DEFAULT;
}
#endif

QCursorData::QCursorData(Qt::CursorShape s)
    : cshape(s), bm(0), bmm(0), hx(0), hy(0)
{
	if(s == Qt::IBeamCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_I_BEAM])
			HaikuCursorCache[B_CURSOR_ID_I_BEAM] = new BCursor(B_CURSOR_ID_I_BEAM);		
	if(s == Qt::PointingHandCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_FOLLOW_LINK])
			HaikuCursorCache[B_CURSOR_ID_FOLLOW_LINK] = new BCursor(B_CURSOR_ID_FOLLOW_LINK);
	if(s == Qt::CrossCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_CROSS_HAIR])
			HaikuCursorCache[B_CURSOR_ID_CROSS_HAIR] = new BCursor(B_CURSOR_ID_CROSS_HAIR);
	if(s == Qt::SplitHCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_RESIZE_EAST_WEST])
			HaikuCursorCache[B_CURSOR_ID_RESIZE_EAST_WEST] = new BCursor(B_CURSOR_ID_RESIZE_EAST_WEST);
	if(s == Qt::SplitVCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_RESIZE_NORTH_SOUTH])
			HaikuCursorCache[B_CURSOR_ID_RESIZE_NORTH_SOUTH] = new BCursor(B_CURSOR_ID_RESIZE_NORTH_SOUTH);
	if(s == Qt::BlankCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_NO_CURSOR])
			HaikuCursorCache[B_CURSOR_ID_NO_CURSOR] = new BCursor(B_CURSOR_ID_NO_CURSOR);
	if(s == Qt::WhatsThisCursor)
		if(!HaikuCursorCache[B_CURSOR_ID_HELP])
			HaikuCursorCache[B_CURSOR_ID_HELP] = new BCursor(B_CURSOR_ID_HELP);
	
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
