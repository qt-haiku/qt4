#include "qsystemtrayicon.h"
#include "qsystemtrayicon_p.h"

#include <stdio.h>

void QSystemTrayIconPrivate::install_sys()
{
	fprintf(stderr, "Unimplemented: QSystemTrayIconPrivate::install_sys \n");
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &title,  const QString &message, QSystemTrayIcon::MessageIcon type, int timeOut)
{
	Q_UNUSED(title);
	Q_UNUSED(message);
	Q_UNUSED(type);
	Q_UNUSED(timeOut);
	fprintf(stderr, "Unimplemented:  QSystemTrayIconPrivate::showMessage_sys\n");
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
	fprintf(stderr, "Unimplemented: QSystemTrayIconPrivate::geometry_sys \n");
	return QRect();
}

void QSystemTrayIconPrivate::remove_sys()
{
	fprintf(stderr, "Unimplemented: QSystemTrayIconPrivate::remove_sys \n");
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
	fprintf(stderr, "Unimplemented:  QSystemTrayIconPrivate::updateIcon_sys\n");
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
	fprintf(stderr, "Unimplemented:  QSystemTrayIconPrivate::updateMenu_sys\n");
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
	fprintf(stderr, "Unimplemented:  QSystemTrayIconPrivate::updateToolTip_sys\n");
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
	fprintf(stderr, "Unimplemented:  QSystemTrayIconPrivate::isSystemTrayAvailable_sys\n");
	return false;
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
    return false;
}
