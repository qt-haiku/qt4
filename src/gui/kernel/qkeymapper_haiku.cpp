/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights.  These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qkeymapper_p.h"

#include <stdio.h>

QKeyMapperPrivate::QKeyMapperPrivate()
{
 uint32 scan_codes[] = {
        Qt::Key_Escape,		0x01,
        Qt::Key_F1,			0x02,
        Qt::Key_F2,			0x03,
        Qt::Key_F3,			0x04,
        Qt::Key_F4,			0x05,
        Qt::Key_F5,			0x06,
        Qt::Key_F6,			0x07,
        Qt::Key_F7,			0x08,
        Qt::Key_F8,			0x09,
        Qt::Key_F9,			0x0A,
        Qt::Key_F10,		0x0B,
        Qt::Key_F11,		0x0C,
        Qt::Key_F12,		0x0D,
        Qt::Key_Print,		0x0E,
//      Qt::Key_ScrollLock = 0x0F,  //modificator
        Qt::Key_Pause,		0x22,

		Qt::Key_AsciiTilde, 0x11,
        Qt::Key_1,			0x12,
        Qt::Key_2,			0x13,
        Qt::Key_3,			0x14,
        Qt::Key_4,			0x15,
        Qt::Key_5,			0x16,
        Qt::Key_6,			0x17,
        Qt::Key_7,			0x18,
        Qt::Key_8,			0x19,
        Qt::Key_9,			0x1A,
        Qt::Key_0,			0x1B,
        Qt::Key_Minus,		0x1C,
        Qt::Key_Plus,		0x1D,
        Qt::Key_Backspace,	0x1E,
        Qt::Key_Insert,		0x1F,
        Qt::Key_Home,		0x20,
        Qt::Key_PageUp,		0x21,
//		Qt::Key_NumLock,	0x22,   //modificator    
		Qt::Key_Slash,		0x23,
		Qt::Key_Asterisk,	0x24,
		Qt::Key_Minus,		0x25,		
		                
        Qt::Key_Tab,		0x26,        
        Qt::Key_Q,			0x27,
        Qt::Key_W,			0x28,
        Qt::Key_E,			0x29,
        Qt::Key_R,			0x2A,
        Qt::Key_T,			0x2B,
        Qt::Key_Y,			0x2C,
        Qt::Key_U,			0x2D,
        Qt::Key_I,			0x2E,
        Qt::Key_O,			0x2F,
        Qt::Key_P,			0x30,      
        Qt::Key_BracketLeft,0x31,
        Qt::Key_BracketRight,0x32,
		Qt::Key_Backslash,	0x33,
        Qt::Key_Delete,		0x34,
        Qt::Key_End,		0x35,
        Qt::Key_PageDown,	0x36, 
		Qt::Key_Home,		0x37, //numpad		      
        Qt::Key_Up,			0x38, //numpad
        Qt::Key_PageUp,		0x39, //numpad
        Qt::Key_Plus,		0x3A, //numpad

//		Qt::Key_CapsLock,	0x3B, //modificator
        Qt::Key_A,			0x3C,
        Qt::Key_S,			0x3D,
        Qt::Key_D,			0x3E,
        Qt::Key_F,			0x3F,
        Qt::Key_G,			0x40,
        Qt::Key_H,			0x41,
        Qt::Key_J,			0x42,
        Qt::Key_K,			0x43,
        Qt::Key_L,			0x44,
        Qt::Key_Colon,		0x45,
        Qt::Key_QuoteDbl,	0x46,
        Qt::Key_Return,		0x47,              
        Qt::Key_Left,		0x48, //numpad
		Qt::Key_5,			0x49, //numpad ???
        Qt::Key_Right,		0x4A, //numpad

        Qt::Key_Z,			0x4C,
        Qt::Key_X,			0x4D,
        Qt::Key_C,			0x4E,
        Qt::Key_V,			0x4F,
        Qt::Key_B,			0x50,
        Qt::Key_N,			0x51,
        Qt::Key_M,			0x51,
        Qt::Key_Less,		0x52,
        Qt::Key_Greater,	0x54,
        Qt::Key_Question,	0x55,
        Qt::Key_Up,			0x57,	//cursor
        Qt::Key_End,		0x58,	//numpad
        Qt::Key_Down,		0x59,   //numpad
        Qt::Key_PageDown,	0x5A,   //numpad
		Qt::Key_Enter,		0x5B,   //numpad

		Qt::Key_Space,		0x5E,
		Qt::Key_Left,		0x61,   //cursor
		Qt::Key_Down,		0x62,   //cursor
		Qt::Key_Right,		0x63,   //cursor
		Qt::Key_Insert,		0x64,   //cursor
		Qt::Key_Delete,		0x65,   //cursor
		0,					0x00
	};
	memcpy(ScanCodes,scan_codes,sizeof(scan_codes));
}

QKeyMapperPrivate::~QKeyMapperPrivate()
{ }

void QKeyMapperPrivate::clearMappings()
{ }

QList<int> QKeyMapperPrivate::possibleKeys(QKeyEvent *)
{
	return QList<int>();
}

uint32 QKeyMapperPrivate::translateKeyCode(int32 key)
{
	uint32 code = 0;
	int i = 0;
    while (ScanCodes[i]) {
      		if ( key == ScanCodes[i+1]) {
            code = ScanCodes[i];
      		    break;
        }
       	i += 2;
  		}	
	return code;   
}
