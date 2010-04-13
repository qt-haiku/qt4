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

#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpaintdevice.h"
#include "qstring.h"
#include <private/qtextengine_p.h>
#include <private/qunicodetables_p.h>
#include <qapplication.h>
#include "qfontdatabase.h"
#include <qpainter.h>
#include "qtextengine_p.h"
#include <stdlib.h>

QT_BEGIN_NAMESPACE

float qt_haiku_defaultDpi_x()
{
	return 72.0;
}

int qt_haiku_pixelsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pixelSize == -1)
        ret = def.pointSize *  dpi / qt_haiku_defaultDpi_x();
    else
        ret = def.pixelSize;
    return qRound(ret);
}

int qt_haiku_pointsize(const QFontDef &def, int dpi)
{
    float ret;
    if(def.pointSize < 0)
        ret = def.pixelSize * qt_haiku_defaultDpi_x() / float(dpi);
    else
        ret = def.pointSize;
    return qRound(ret);
}

Q_GUI_EXPORT const char *qt_fontFamilyFromStyleHint(const QFontDef &request)
{
    const char *family = 0;
    switch (request.styleHint) {
    case QFont::Helvetica:
        family = "DejaVu Sans";
        break;
    case QFont::Times:
    case QFont::OldEnglish:
        family = "DejaVu Serif";
        break;
    case QFont::Courier:
        family = "DejaVu Sans Mono";
        break;
    case QFont::System:
        family = "DejaVu Sans";
        break;
    case QFont::AnyStyle:
        if (request.fixedPitch)
            family = "DejaVu Sans Mono";
        else
            family = "DejaVu Sans";
        break;
    }
    return family;
}

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
}

void QFont::cleanup()
{
    QFontCache::cleanup();
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName(const QString &name)
{
    setFamily(name);
}

QString QFont::defaultFamily() const
{
    return QLatin1String(qt_fontFamilyFromStyleHint(d->request));
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("DejaVu Sans");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("DejaVu Sans");
}

QT_END_NAMESPACE
