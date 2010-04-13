/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
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
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcolormap.h"

#include "qapplication.h"
#include "qdebug.h"
#include "qdesktopwidget.h"
#include "qvarlengtharray.h"

#include <limits.h>

QT_BEGIN_NAMESPACE

class QColormapPrivate
{
public:
    QColormapPrivate()
        : ref(1), mode(QColormap::Direct), depth(0),
          defaultColormap(true),
          defaultVisual(true),
          r_max(0), g_max(0), b_max(0),
          r_shift(0), g_shift(0), b_shift(0)
    {}

    QAtomicInt ref;

    QColormap::Mode mode;
    int depth;

//    Colormap colormap;
    bool defaultColormap;

//    Visual *visual;
    bool defaultVisual;

    int r_max;
    int g_max;
    int b_max;

    uint r_shift;
    uint g_shift;
    uint b_shift;

    QVector<QColor> colors;
    QVector<int> pixels;
};


static QColormap **cmaps = 0;

/*! \internal
*/
void QColormap::initialize()
{
}

/*! \internal
*/
void QColormap::cleanup()
{
 //   Display *display = QX11Info::display();
    const int screens = 0;//ScreenCount(display);

    for (int i = 0; i < screens; ++i)
        delete cmaps[i];

    delete [] cmaps;
    cmaps = 0;
}

QColormap QColormap::instance(int screen)
{
    //if (screen == -1)
      //  screen = QX11Info::appScreen();
      QColormap cm;
    return cm;//*cmaps[screen];
}

/*! \internal
    Constructs a new colormap.
*/
QColormap::QColormap()
    : d(new QColormapPrivate)
{}

QColormap::QColormap(const QColormap &colormap)
    :d (colormap.d)
{ d->ref.ref(); }

QColormap::~QColormap()
{
    if (!d->ref.deref()) {
        if (!d->defaultColormap) {}
//            XFreeColormap(QX11Info::display(), d->colormap);
        delete d;
    }
}

QColormap::Mode QColormap::mode() const
{ return d->mode; }

int QColormap::depth() const
{ return d->depth; }

int QColormap::size() const
{
    return (d->mode == Gray
            ? d->r_max
            : (d->mode == Indexed
               ? d->r_max * d->g_max * d->b_max
               : -1));
}

uint QColormap::pixel(const QColor &color) const
{
    const QColor c = color.toRgb();
    const uint r = (c.ct.argb.red   * d->r_max) >> 16;
    const uint g = (c.ct.argb.green * d->g_max) >> 16;
    const uint b = (c.ct.argb.blue  * d->b_max) >> 16;
    if (d->mode != Direct) {
        if (d->mode == Gray)
            return d->pixels.at((r * 30 + g * 59 + b * 11) / 100);
        return d->pixels.at(r * d->g_max * d->b_max + g * d->b_max + b);
    }
    return (r << d->r_shift) + (g << d->g_shift) + (b << d->b_shift);
}

const QColor QColormap::colorAt(uint pixel) const
{
    if (d->mode != Direct) {
        Q_ASSERT(pixel <= (uint)d->colors.size());
        return d->colors.at(pixel);
    }

    //const int r = (((pixel & d->visual->red_mask)   >> d->r_shift) << 8) / d->r_max;
    //const int g = (((pixel & d->visual->green_mask) >> d->g_shift) << 8) / d->g_max;
    //const int b = (((pixel & d->visual->blue_mask)  >> d->b_shift) << 8) / d->b_max;
    return QColor(0, 0, 0);
}

const QVector<QColor> QColormap::colormap() const
{ return d->colors; }

QColormap &QColormap::operator=(const QColormap &colormap)
{
    qAtomicAssign(d, colormap.d);
    return *this;
}

QT_END_NAMESPACE
