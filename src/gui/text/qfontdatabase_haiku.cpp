#include "qfontdatabase.h"
#include <qabstractfileengine.h> //to get qfontdatabase.cpp compiling
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qfontengine_ft_p.h"
#include "qfontengine_haiku_p.h"
#include "qpaintdevice.h"
#include "qlibrary.h"
#include "qabstractfileengine.h"
#include "qendian.h"
#include "qfontdatabase.h"

#include <stdio.h>

#include <Path.h>
#include <Entry.h>
#include <File.h>
#include <Directory.h>
#include <FindDirectory.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPES_H
#include FT_TRUETYPE_TABLES_H
#include FT_LCD_FILTER_H

struct FaceData
{
    int index;
    QString familyName;
    QtFontStyle::Key styleKey;
    QList<QFontDatabase::WritingSystem> systems;
    bool fixedPitch;
    bool smoothScalable;
    QList<unsigned short> pixelSizes;
};

// from qfont_haiku.cpp
extern int qt_haiku_pixelsize(const QFontDef &def, int dpi);
extern int qt_haiku_pointsize(const QFontDef &def, int dpi);
extern float qt_haiku_defaultDpi_x();

static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if(!db || db->count)
        return;

    extern FT_Library qt_getFreetype(); // qfontengine_ft.cpp
    FT_Library lib = qt_getFreetype();

	directory_which FontDirs[] = {
		B_BEOS_FONTS_DIRECTORY,
		B_COMMON_FONTS_DIRECTORY,
		B_USER_FONTS_DIRECTORY
	};
	
	
	for(int nfont=0;nfont<3;nfont++)
	{	
		BPath fontDir;
		find_directory(FontDirs[nfont],&fontDir);
		fontDir.Append("ttfonts");
		
		BDirectory dir(fontDir.Path());
		if (dir.InitCheck() != B_OK)
			continue;
			
	 	BEntry ent;
	 	char filename[B_FILE_NAME_LENGTH];
	 	int file_index_value = 0;
	
	 	while (dir.GetNextEntry(&ent) == B_NO_ERROR)
	 	{
	 		QList<FaceData> facedata;
	
			ent.GetName(filename);
	 		BPath path = BPath(&dir, filename);	
			
	        FT_Long numFaces = 0;
	        FT_Face face;
	
	        FT_Error rc = FT_New_Face(lib, path.Path(), -1, &face);
	        if (rc == 0) {
	            numFaces = face->num_faces; 
	        	FT_Done_Face(face);                                   
	        }
	        
	        for (FT_Long idx = 0; idx < numFaces; ++idx) {
	                rc = FT_New_Face(lib, path.Path(), idx, &face);
	                FaceData fdata;
	                fdata.index = idx;
	                fdata.familyName = QString::fromLatin1(face->family_name);
	                fdata.familyName = fdata.familyName.trimmed();
	                
	                fdata.styleKey.style = face->style_flags & FT_STYLE_FLAG_ITALIC ? QFont::StyleItalic : QFont::StyleNormal;
	
	                TT_OS2 *os2_table = 0;
	                if (face->face_flags & FT_FACE_FLAG_SFNT) {
	                    os2_table = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
	                }
	                if (os2_table) {
	                    // map weight and width values
	                    if (os2_table->usWeightClass < 400)
	                        fdata.styleKey.weight = QFont::Light;
	                    else if (os2_table->usWeightClass < 600)
	                        fdata.styleKey.weight = QFont::Normal;
	                    else if (os2_table->usWeightClass < 700)
	                        fdata.styleKey.weight = QFont::DemiBold;
	                    else if (os2_table->usWeightClass < 800)
	                        fdata.styleKey.weight = QFont::Bold;
	                    else
	                        fdata.styleKey.weight = QFont::Black;
	
	                    switch (os2_table->usWidthClass) {
	                        case 1: fdata.styleKey.stretch = QFont::UltraCondensed; break;
	                        case 2: fdata.styleKey.stretch = QFont::ExtraCondensed; break;
	                        case 3: fdata.styleKey.stretch = QFont::Condensed; break;
	                        case 4: fdata.styleKey.stretch = QFont::SemiCondensed; break;
	                        case 5: fdata.styleKey.stretch = QFont::Unstretched; break;
	                        case 6: fdata.styleKey.stretch = QFont::SemiExpanded; break;
	                        case 7: fdata.styleKey.stretch = QFont::Expanded; break;
	                        case 8: fdata.styleKey.stretch = QFont::ExtraExpanded; break;
	                        case 9: fdata.styleKey.stretch = QFont::UltraExpanded; break;
	                        default: fdata.styleKey.stretch = QFont::Unstretched; break;
	                    }                  
	                    quint32 unicodeRange[4] = {
	                        os2_table->ulUnicodeRange1, os2_table->ulUnicodeRange2,
	                        os2_table->ulUnicodeRange3, os2_table->ulUnicodeRange4
	                    };
	                    quint32 codePageRange[2] = {
	                        os2_table->ulCodePageRange1, os2_table->ulCodePageRange2
	                    };
	                    fdata.systems = qt_determine_writing_systems_from_truetype_bits(unicodeRange, codePageRange);                          
	                } else {
	                    fdata.styleKey.weight = face->style_flags & FT_STYLE_FLAG_BOLD ? QFont::Bold : QFont::Normal;
	                    fdata.styleKey.stretch = QFont::Unstretched;
	                }
	                
	                fdata.fixedPitch = face->face_flags & FT_FACE_FLAG_FIXED_WIDTH;
	                fdata.smoothScalable = face->face_flags & FT_FACE_FLAG_SCALABLE;
	
	                if (face->face_flags & FT_FACE_FLAG_FIXED_SIZES) {
	                    for (FT_Int i = 0; i < face->num_fixed_sizes; ++i) {
	                        fdata.pixelSizes << face->available_sizes[i].height;
	                    }
	                }
	            	facedata << fdata;            	
	    		
	    		    if(face)
		        		FT_Done_Face(face);            	
	        }
	     
	      	foreach(FaceData cached, facedata) {
		 		const QString foundryName;
		 		QtFontFamily *family = privateDb()->family(cached.familyName, true);
		 		family->fixedPitch = cached.fixedPitch;
		 		
		 		for (int i = 0; i < cached.systems.count(); ++i)
	               family->writingSystems[cached.systems.at(i)] = QtFontFamily::Supported;
		 		
		 		QtFontFoundry *foundry = family->foundry(foundryName, true);
		 		QtFontStyle *style = foundry->style(cached.styleKey, QString(), true);
	
		 		style->antialiased = true;
	
		        QByteArray file((const char *)path.Path());
	            
	            if (cached.smoothScalable && !style->smoothScalable) {
	                style->smoothScalable = true;
	                QtFontSize *size = style->pixelSize(SMOOTH_SCALABLE, true);
	                size->fileName = file;
	                size->fileIndex = cached.index;
	            }
		        
	            foreach(unsigned short pixelSize, cached.pixelSizes) {
	                QtFontSize *size = style->pixelSize(pixelSize, true);
	                // the first bitmap style with a given pixel and point size wins
	                if (!size->fileName.isEmpty())
	                    continue;
	                size->fileName = file;
	                size->fileIndex = cached.index;
	            }            
	 		} 
	 		file_index_value++;
	 	}
	}
}

static inline void load(const QString &family = QString(), int = -1)
{                
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
}

static QFontDef fontDescToFontDef(const QFontDef &req, const QtFontDesc &desc)
{
    static long dpi = qt_haiku_defaultDpi_x();

    QFontDef fontDef;

    fontDef.family = desc.family->name;

    if (desc.size->pixelSize == SMOOTH_SCALABLE) {
        // scalable font matched, calculate the missing size (points or pixels)
        fontDef.pointSize = req.pointSize;
        fontDef.pixelSize = req.pixelSize;
        if (req.pointSize < 0) {
            fontDef.pointSize = req.pixelSize * 72. / dpi;
        } else if (req.pixelSize == -1) {
            fontDef.pixelSize = qRound(req.pointSize * dpi / 72.);
        }
    } else {
        // non-scalable font matched, calculate both point and pixel size
        fontDef.pixelSize = desc.size->pixelSize;
        fontDef.pointSize = desc.size->pixelSize * 72. / dpi;
    }

    fontDef.styleStrategy = req.styleStrategy;
    fontDef.styleHint = req.styleHint;

    fontDef.weight = desc.style->key.weight;
    fontDef.fixedPitch = desc.family->fixedPitch;
    fontDef.style = desc.style->key.style;
    fontDef.stretch = desc.style->key.stretch;

    return fontDef;
}

static QFontEngine *loadEngine(const QFontDef &req, const QtFontDesc &desc)
{
    // @todo all these fixed so far; make configurable through the Registry
    // on per-family basis
    QFontEngineHaiku::HintStyle hintStyle = QFontEngineHaiku::HintFull;
    bool autoHint = true;
    QFontEngineFT::SubpixelAntialiasingType subPixel = QFontEngineFT::Subpixel_None;
    int lcdFilter = FT_LCD_FILTER_DEFAULT;
    bool useEmbeddedBitmap = true;

    QFontEngine::FaceId faceId;
    faceId.filename = desc.size->fileName;
    faceId.index = desc.size->fileIndex;

    QFontEngineFT *fe = new QFontEngineHaiku(fontDescToFontDef(req, desc), faceId,
                                            desc.style->antialiased, hintStyle,
                                            autoHint, subPixel, lcdFilter,
                                            useEmbeddedBitmap);
    Q_ASSERT(fe);
    if (fe && fe->invalid()) {
        FM_DEBUG("   --> invalid!\n");
        delete fe;
        fe = 0;
    }
    return fe;
}

static QFontEngine *loadHaikuFT(const QFontPrivate *d, int script, const QFontDef &req)
{
    // list of families to try
    QStringList families = familyList(req);

    const char *styleHint = qt_fontFamilyFromStyleHint(d->request);
    if (styleHint)
        families << QLatin1String(styleHint);

    // add the default family
    QString defaultFamily = QApplication::font().family();
    if (! families.contains(defaultFamily))
        families << defaultFamily;

    // add QFont::defaultFamily() to the list, for compatibility with
    // previous versions
    families << QApplication::font().defaultFamily();

    // null family means find the first font matching the specified script
    families << QString();

    QtFontDesc desc;
    QFontEngine *fe = 0;
    QList<int> blacklistedFamilies;

    while (!fe) {
        for (int i = 0; i < families.size(); ++i) {
            QString family, foundry;
            parseFontName(families.at(i), foundry, family);
            QT_PREPEND_NAMESPACE(match)(script, req, family, foundry, -1, &desc, blacklistedFamilies);
            if (desc.family)
                break;
        }
        if (!desc.family)
            break;
        fe = loadEngine(req, desc);
        if (!fe)
            blacklistedFamilies.append(desc.familyIndex);
    }
    return fe;
}

void QFontDatabase::load(const QFontPrivate *d, int script)
{
    Q_ASSERT(script >= 0 && script < QUnicodeTables::ScriptCount);

    // normalize the request to get better caching
    QFontDef req = d->request;

    if (req.pixelSize <= 0)
        req.pixelSize = qMax(1, qRound(req.pointSize * d->dpi / qt_haiku_defaultDpi_x()));
    req.pointSize = 0;
    if (req.weight == 0)
        req.weight = QFont::Normal;
    if (req.stretch == 0)
        req.stretch = 100;

    QFontCache::Key key(req, d->rawMode ? QUnicodeTables::Common : script, d->screen);
    if (!d->engineData)
        getEngineData(d, key);

    // the cached engineData could have already loaded the engine we want
    if (d->engineData->engines[script])
        return;

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = req.pixelSize * qt_haiku_defaultDpi_x() / d->dpi;

    QFontEngine *fe = QFontCache::instance()->findEngine(key);

 //   qDebug()<<"QFontDatabase::load(2) pixelsize:"<<req.pixelSize <<" pointSize:"<< req.pointSize<< " weight:"<< req.weight<< " family:"<< req.family ;

    if (!fe) {
        if (qt_enable_test_font && req.family == QLatin1String("__Qt__Box__Engine__")) {
            fe = new QTestFontEngine(req.pixelSize);
            fe->fontDef = req;
        } else {
            QMutexLocker locker(fontDatabaseMutex());
            if (!privateDb()->count)
                initializeDb();
            fe = loadHaikuFT(d, script, req);
        }
        if (!fe) {
            fe = new QFontEngineBox(req.pixelSize);
            fe->fontDef = QFontDef();
        }
    }
    if (fe->symbol || (d->request.styleStrategy & QFont::NoFontMerging)) {
        for (int i = 0; i < QUnicodeTables::ScriptCount; ++i) {
            if (!d->engineData->engines[i]) {
                d->engineData->engines[i] = fe;
                fe->ref.ref();
            }
        }
    } else {
        d->engineData->engines[script] = fe;
        fe->ref.ref();
    }
    QFontCache::instance()->insertEngine(key, fe);
}


bool QFontDatabase::removeApplicationFont(int handle)
{
    return false;
}

bool QFontDatabase::removeAllApplicationFonts()
{
    return false;
}

bool QFontDatabase::supportsThreadedFontRendering()
{
    return true;
}
