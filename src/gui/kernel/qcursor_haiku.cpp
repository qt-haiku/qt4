#include "qcursor.h"
#include "qcursor_p.h"

#include <Application.h>
#include <Cursor.h>

#include <stdio.h>

int	haiku_global_mouse_x = 0;
int	haiku_global_mouse_y = 0;

static const unsigned char cursor_haiku_pointing_hand_bmp[] =	 {
	16,1,2,2,
		
	0,0,	0,0,	56,0,	36,0,	36,00,	19,224,	18,92,	9,42,
	8,1,	60,33,	76,113,	66,113,	48,249,	12,249,	2,0,	1,0,
		
	0,0,	0,0,	56,0,	60,0,	60,0,	31,224,	31,252,	15,254,
	15,255,	63,255,	127,255,127,255,63,255,	15,255,	3,254,	1,248
};

static const unsigned char cursor_haiku_h_resize_bmp[] =
{
	16,1,7,7,
	
	0,0,	1,0,	1,0,	1,0,	1,0,	17,16,	49,24,	113,28,	
	49,24,	17,16,	1,0,	1,0,	1,0,	1,0,	0,0,	0,0,
	
	3,128,	3,128,	3,128,	3,128,	27,176,	59,184,	123,188,251,190,
	123,188,59,184,	27,176,	3,128,	3,128,	3,128,	3,128,	0,0
};

static const unsigned char cursor_haiku_v_resize_bmp[] =
{
	16,1,7,7,
	0,0,	1,0,	3,128,	7,192,	0,0,	0,0,	0,0,	127,252,
	0,0,	0,0,	0,0,	7,192,	3,128,	1,0,	0,0,	0,0,

	1,0,	3,128,	7,192,	15,224,	15,224,	0,0,	255,254,255,254,
	255,254,0,0,	15,224,	15,224,	7,192,	3,128,	1,0,	0,0
};

static const unsigned char cursor_haiku_crosshairs_bmp[] =
{
	16,1,7,7,
	
	0,0,	1,0,	1,0,	1,0,	1,0,	1,0,	0,0,	125,124,
	0,0,	1,0,	1,0,	1,0,	1,0,	1,0,	0,0,	0,0,

	3,128,	3,128,	3,128,	3,128,	3,128,	3,128,	255,254,255,254,
	255,254,3,128,	3,128,	3,128,	3,128,	3,128,	3,128,	0,0
};

BCursor *B_CURSOR_HAIKU_POINTING_HAND = NULL;
BCursor *B_CURSOR_HAIKU_SPLIT_V = NULL;
BCursor *B_CURSOR_HAIKU_SPLIT_H = NULL;
BCursor *B_CURSOR_HAIKU_CROSS = NULL;

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
		return (BCursor*)B_CURSOR_I_BEAM;
	else if(d->cshape==Qt::PointingHandCursor)
		return B_CURSOR_HAIKU_POINTING_HAND;
	else if(d->cshape==Qt::SplitVCursor)
		return B_CURSOR_HAIKU_SPLIT_V;
	else if(d->cshape==Qt::SplitHCursor)
		return B_CURSOR_HAIKU_SPLIT_H;
	else if(d->cshape==Qt::CrossCursor)
		return B_CURSOR_HAIKU_CROSS;
	else
		return (BCursor*)B_CURSOR_SYSTEM_DEFAULT;
}
#endif

QCursorData::QCursorData(Qt::CursorShape s)
    : cshape(s), bm(0), bmm(0), hx(0), hy(0)
{
	if(s == Qt::PointingHandCursor)
		if(!B_CURSOR_HAIKU_POINTING_HAND)
			B_CURSOR_HAIKU_POINTING_HAND = new BCursor(cursor_haiku_pointing_hand_bmp);	
	if(s == Qt::SplitVCursor)
		if(!B_CURSOR_HAIKU_SPLIT_V)
			B_CURSOR_HAIKU_SPLIT_V = new BCursor(cursor_haiku_v_resize_bmp);	
	if(s == Qt::SplitHCursor)
		if(!B_CURSOR_HAIKU_SPLIT_H)
			B_CURSOR_HAIKU_SPLIT_H = new BCursor(cursor_haiku_h_resize_bmp);	
	if(s == Qt::CrossCursor)
		if(!B_CURSOR_HAIKU_CROSS)
			B_CURSOR_HAIKU_CROSS = new BCursor(cursor_haiku_crosshairs_bmp);	
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
