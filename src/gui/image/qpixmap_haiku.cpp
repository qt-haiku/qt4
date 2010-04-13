#include "qpixmap.h"

#include <stdio.h>

#include <View.h>
#include <Window.h>
#include <Screen.h>
#include <Bitmap.h>

QPixmap QPixmap::grabWindow(WId winId, int x, int y, int w, int h )
{
    if (w == 0 || h == 0 || winId!=0)
        return QPixmap();
        	
	BScreen screen(NULL);
	BBitmap *bitmap=NULL;
	screen.GetBitmap(&bitmap);

	int screen_w = screen.Frame().Width()+1;
	int screen_h = screen.Frame().Height()+1;	

    if (w < 0)
        w = screen_w - x;
    if (h < 0)
        h = screen_h - y;

	QImage::Format format = QImage::Format_RGB32;
	switch(bitmap->ColorSpace()) {
		case B_GRAY1:
			format = QImage::Format_Mono;
			break;
		case B_GRAY8:
		case B_CMAP8:
			format = QImage::Format_Indexed8;
			break;
		case B_RGB15:
		case B_RGBA15:
		case B_RGB16:
			format = QImage::Format_RGB16;
			break;
		case B_RGB32:
		default:
			format = QImage::Format_RGB32;
			break;
	}

	QRect grabRect(x,y,w,h);
	QImage image((uchar*)bitmap->Bits(), screen_w, screen_h, bitmap->BytesPerRow(), format);
    image = image.copy(grabRect);
    
	delete bitmap;
	
	return QPixmap::fromImage(image);
}

