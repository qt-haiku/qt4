#include "qpixmap.h"

#include "qpixmap_raster_p.h"

#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qapplication.h"
#include "qevent.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdatetime.h"
#include "qpixmapcache.h"
#include "qimagereader.h"
#include "qimagewriter.h"
#include "qdebug.h"

#include <stdio.h>

#include <View.h>
#include <Window.h>
#include <Screen.h>
#include <Bitmap.h>

BBitmap *
QPixmap::toHaikuBitmap() const
{
	BBitmap *bitmap = NULL;
	
    if (isNull())
        return 0;
    
    if (data->classId() == QPixmapData::RasterClass) {
        QRasterPixmapData* d = static_cast<QRasterPixmapData*>(data.data());
        int w = d->image.width();
        int h = d->image.height();

        const QImage image = d->image.convertToFormat(QImage::Format_ARGB32);
        int bytes_per_line = w * 4;        
        
        bitmap = new BBitmap(BRect(0,0,w-1,h-1), B_RGBA32);
        uchar *pixels = (uchar *)bitmap->Bits();

        for (int y=0; y<h; ++y)
            memcpy(pixels + y * bytes_per_line, image.scanLine(y), bytes_per_line);
	
	} else {
    
        QPixmapData *data = new QRasterPixmapData(depth() == 1 ?
                                                  QPixmapData::BitmapType : QPixmapData::PixmapType);
        data->fromImage(toImage(), Qt::AutoColor);
        return QPixmap(data).toHaikuBitmap();
    }        	
	return bitmap;
}

QPixmap
QPixmap::fromHaikuBitmap(BBitmap *bmp)
{
	if(!bmp) 
		return QPixmap();
		
	int w = bmp->Bounds().IntegerWidth() + 1;
	int h = bmp->Bounds().IntegerHeight() + 1;
	
	QImage image(w,h,QImage::Format_ARGB32);

	int bytes_per_line = w * 4;                        	
    uchar *pixels = (uchar *)bmp->Bits();

    for (int y=0; y<h; ++y)
       memcpy( image.scanLine(y), pixels + y * bytes_per_line, bytes_per_line);
    
	return QPixmap::fromImage(image);
}

QPixmap 
QPixmap::grabWindow(WId winId, int x, int y, int w, int h )
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

