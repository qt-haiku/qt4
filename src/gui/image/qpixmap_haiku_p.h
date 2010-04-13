#include "qpixmap.h"
#include "qpixmapdata_p.h"

#include <stdio.h>


// ### TODO: remove me
class QHaikuPixmapData : public QPixmapData
{
public:
//QHaikuPixmapData()
//	{
//	fprintf(stderr, "Unimplemented: QHaikuPixmapData class\n");
//	}
QHaikuPixmapData(QPixmapData::PixelType type, int classId id)
: QPixmapData(type, id)
{
	fprintf(stderr, "Unimplemented:  
	HaikuPixmapData(QPixmapData::PixelType)\n");
}

virtual void resize(int width, int height)
{
	fprintf(stderr, "Unimplemented: QHaikuPixmapData::resize\n");
}

    virtual void fromImage(const QImage &image,
                           Qt::ImageConversionFlags flags)
	{
		fprintf(stderr, "Unimplemented: QHaikuPixmapData::fromImage\n");
	}

    virtual int metric(QPaintDevice::PaintDeviceMetric metric)
	{
		fprintf(stderr, "Unimplemented: QHaikuPixmapData::metric\n");
		return 0;
	}

	virtual void fill(const QColor &color)
	{
		fprintf(stderr, "Unimplemented: QHaikuPixmapData::fill\n");
	}
	virtual bool hasAlphaChannel() const
	{
		fprintf(stderr, "Unimplemented: QHaikuPixmapData::hasAlphaChannel\n");
		return false;
	}

    virtual QImage toImage() const
	{
		fprintf(stderr, "Unimplemented: QHaikuPixmapData::toImage\n");
		return QImage();
	}

    virtual QPaintEngine* paintEngine() const
	{
		fprintf(stderr, "Unimplemented: QHaikuPixmapData::paintEngine\n");
		return 0;
	}

};
