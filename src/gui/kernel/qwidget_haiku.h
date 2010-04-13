#ifndef QWIDGET_HAIKU_H
#define QWIDGET_HAIKU_H

#include <Window.h>
#include <View.h>
#include <Message.h>
#include "qevent.h"

#include "qwidget.h"
//#include "qwidget_p.h"
//#include "qtextcodec.h"

//#include <stdio.h>
//#include "qevent_p.h"
//#include "private/qwindowsurface_raster_p.h"
//#include "qapplication.h"
//#include "qfileinfo.h"


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QWidgetPrivate;

class QtHaikuView :  public QObject, public BView
{
	Q_OBJECT
	
public:
	QtHaikuView(BRect frame, const char *name, uint32 resizingMode,  uint32 flags);
	~QtHaikuView();

	virtual void MessageReceived(BMessage* msg);
	virtual void FrameResized(float width, float height);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	virtual void MouseMoved(BPoint where, uint32 code, const BMessage *message);
	virtual void Hide();
	virtual void Show();
	virtual void Draw(BRect rect);

	inline void setWidget(QWidget *widget, QWidgetPrivate *widgetPrivate);
	void sendWheelEvent(int delta_x, int delta_y);
	
	Qt::KeyboardModifiers translateModifiers(uint32 s);
	bool isVisible;		
Q_SIGNALS:
	void sendHaikuEvent(QObject *receiver, QEvent *event);
	void sendUpdateWidget();
	void sendUpdateWidgetRect(QRect r);
	void sendResizeWidget(int width, int height);
	void sendMoveWidget(int x, int y);
	void sendSetActiveWindow(QWidget *widget);
	void sendCloseWindow(QWidget *widget);
		
private:
	friend class QtHaikuWindow;
	Qt::MouseButton translateMouseButton(uint32 s);
	Qt::MouseButtons translateMouseButtons(uint32 s);
	
	QWidget* fWidget;
	QWidgetPrivate* fWidgetPrivate;
	
	int	last_down_buttons;
	int last_clicked_buttons;
	int last_mouse_x;
	int last_mouse_y;
	
	bigtime_t last_click_time;
	bigtime_t last_draw_event;
};

class QtHaikuWindow : public BWindow
{
public:
	QtHaikuWindow(BRect frame, const char *title, window_look look, window_feel feel, uint32 flags);
	~QtHaikuWindow();

	void FrameResized(float width, float height);
	void FrameMoved(BPoint point);
	void MessageReceived(BMessage* msg);
	virtual void DispatchMessage(BMessage *, BHandler *);	
	virtual void WindowActivated(bool active);
	virtual bool QuitRequested();
	virtual void Hide();
	virtual void Show();

	inline void setView(QtHaikuView *view);

	void sendKeyEvent(QEvent::Type type, BMessage *msg);
	bool isVisible;		
	bool inDestroy;
	void updateWindowFlags(Qt::WindowFlags f);
private:
	QtHaikuView *fView;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
