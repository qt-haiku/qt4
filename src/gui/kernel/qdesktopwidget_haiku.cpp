#include "qdesktopwidget.h"
#include "interface/Screen.h"


#include <stdio.h>

QDesktopWidget::QDesktopWidget()
    : QWidget(0, Qt::Desktop)
{
	//fprintf(stderr, "Unimplemented: QDesktopWidget::QDesktopWidget\n");
}

QDesktopWidget::~QDesktopWidget()
{
	//fprintf(stderr, "Unimplemented: QDesktopWidget::~QDesktopWidget\n");
}

void
QDesktopWidget::resizeEvent(QResizeEvent*)
{
	fprintf(stderr, "Unimplemented: QDesktopWidget::resizeEvent\n");
}

const QRect QDesktopWidget::availableGeometry(int screen) const
{
	BScreen bscreen(B_MAIN_SCREEN_ID);
	BRect nsg  = bscreen.Frame();
	return QRect(nsg.left,nsg.top,nsg.right,nsg.bottom);
}

const QRect QDesktopWidget::screenGeometry(int screen) const
{
	BScreen bscreen(B_MAIN_SCREEN_ID);
	BRect nsg  = bscreen.Frame();
	return QRect(nsg.left,nsg.top,nsg.right,nsg.bottom);
}

int QDesktopWidget::screenNumber(const QWidget *widget) const
{
	Q_UNUSED(widget);
	//fprintf(stderr, "Reimplemented: QDesktopWidget::screenNumber(widget) \n");
	return 0;
}

int QDesktopWidget::screenNumber(const QPoint &point) const
{
	Q_UNUSED(point);
	//fprintf(stderr, "Reimplemented: QDesktopWidget::screenNumber\n");
	return 0;
}

bool QDesktopWidget::isVirtualDesktop() const
{
	//fprintf(stderr, "Unimplemented: QDesktopWidget::isVirtualDesktop\n");
	return true;
}

int QDesktopWidget::primaryScreen() const
{
	//fprintf(stderr, "Unimplemented: QDesktopWidget::primaryScreen\n");
	return 0;
}

int QDesktopWidget::numScreens() const
{
	//fprintf(stderr, "Unimplemented: QDesktopWidget::numScreens\n");
	return 1;
}

QWidget *QDesktopWidget::screen(int /* screen */)
{
	//fprintf(stderr, "Unimplemented: QDesktopWidget::screen\n");
	// It seems that a Qt::WType_Desktop cannot be moved?
	return this;
}
