#include "qdesktopservices.h"
#include "qdir.h"
#include "qurl.h"

#ifndef QT_NO_DESKTOPSERVICES

#include <stdio.h>

QT_BEGIN_NAMESPACE

static bool openDocument(const QUrl &file)
{
	QString cmd = QLatin1String("open ") + file.toString();
	system((cmd.toUtf8()).data());
	return true;
}

static bool launchWebBrowser(const QUrl &url)
{
	QString cmd = QLatin1String("open ") + url.toString();
	system((cmd.toUtf8()).data());
	return true;
}

QString QDesktopServices::storageLocation(StandardLocation type)
{
    if (type == QDesktopServices::HomeLocation)
        return QLatin1String("/boot/home");
    if (type == QDesktopServices::TempLocation)
        return QDir::tempPath();

    if (type == QDesktopServices::CacheLocation) {
            return QDir::homePath() + QLatin1String("/cache");
    }

    if (type == QDesktopServices::DataLocation) {
            return QDir::homePath() + QLatin1String("/data");
    }

    QDir emptyDir;
    QString path;
    switch (type) {
    case DesktopLocation:
        path = QLatin1String("/boot/home/Desktop");
        break;
    case DocumentsLocation:
        path = QLatin1String("/boot/home/Documents");
       break;
    case PicturesLocation:
        path = QLatin1String("/boot/home/Pictures");
        break;

    case FontsLocation:
        path = QLatin1String("/boot/system/data/fonts/ttfonts");
        break;

    case MusicLocation:
        path = QLatin1String("/boot/home/Music");
        break;

    case MoviesLocation:
        path = QLatin1String("/boot/home/Videos");
        break;

    case ApplicationsLocation:
    default:
        break;
    }
    return path;
}

QString QDesktopServices::displayName(StandardLocation type)
{
    Q_UNUSED(type);
    return QString();
}


QT_END_NAMESPACE

#endif // QT_NO_DESKTOPSERVICES
