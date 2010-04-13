/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** Copyright (C) 2009 netlabs.org. OS/2 parts.
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

#ifndef QFONTENGINE_HAIKU_P_H
#define QFONTENGINE_HAIKU_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qfontengine_ft_p.h>

#ifdef QT_NO_FREETYPE
# error "The OS/2 version of Qt requires FreeType support, but QT_NO_FREETYPE " \
        "is defined!"
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FREETYPE

class Q_GUI_EXPORT QFontEngineHaiku : public QFontEngineFT
{
public:
    enum HintStyle {
        HintNone = QFontEngineFT::HintNone,
        HintLight = QFontEngineFT::HintLight,
        HintMedium = QFontEngineFT::HintMedium,
        HintFull = QFontEngineFT::HintFull
    };

    explicit QFontEngineHaiku(const QFontDef &fd, const FaceId &faceId,
                             bool antialias, HintStyle hintStyle,
                             bool autoHint, SubpixelAntialiasingType subPixel,
                             int lcdFilter, bool useEmbeddedBitmap);
    ~QFontEngineHaiku();
};

#endif

QT_END_NAMESPACE

#endif // QFONTENGINE_HAIKU_P_H
