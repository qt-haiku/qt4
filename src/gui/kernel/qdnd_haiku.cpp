#include "qdnd_p.h"

#include <stdio.h>

bool QDropData::hasFormat_sys(const QString &mimeType) const
{
	Q_UNUSED(mimeType);
	fprintf(stderr, "Unimplemented: QDropData::hasFormat_sys\n");
	return false;
}

QStringList QDropData::formats_sys() const
{
	fprintf(stderr, "Unimplemented: QDropData::formats_sys\n");
	return QStringList();
}

QVariant QDropData::retrieveData_sys(const QString &mimeType,  QVariant::Type type) const
{
	Q_UNUSED(mimeType);
	Q_UNUSED(type);
	fprintf(stderr, "Unimplemented: QDropData::retrieveData_sys\n");
	return QVariant();
}

bool QDragManager::eventFilter(QObject *, QEvent *)
{
	fprintf(stderr, "Unimplemented: QDragManager::eventFilter\n");
	return false;
}

void QDragManager::timerEvent(QTimerEvent*)
{
	fprintf(stderr, "Unimplemented: QDragManager::timerEvent\n");
}

void QDragManager::cancel(bool)
{
	fprintf(stderr, "Unimplemented: QDragManager::cancel\n");
}

Qt::DropAction QDragManager::drag(QDrag*)
{
	fprintf(stderr, "Unimplemented: QDragManager::drag\n");
	return Qt::IgnoreAction;
}
