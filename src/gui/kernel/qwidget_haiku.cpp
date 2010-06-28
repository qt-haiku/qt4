#include "qwidget_haiku.h"
#include <qdebug.h>
#include "qapplication_p.h"
#include "private/qkeymapper_p.h"
#include "qdesktopwidget.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "qtextcodec.h"

#include <assert.h>
#include <stdio.h>
#include "qevent_p.h"
#include "private/qwindowsurface_raster_p.h"
#include "qapplication.h"
#include "qfileinfo.h"
#include <Application.h>
#include <Cursor.h>
#include <interface/Screen.h>

extern	int	haiku_global_mouse_x;
extern	int	haiku_global_mouse_y;

static  QWidget *keyboardGrb = 0;
static  QWidget *mouseGrb    = 0;

QtHaikuView::QtHaikuView(BRect frame, const char *name, uint32  resizingMode, uint32 flags)
		:	QObject(), BView(frame, name, resizingMode, flags),
			isVisible(true), fWidget(0), fWidgetPrivate(0), 
			last_down_buttons(0), last_clicked_buttons(0), last_mouse_x(-1), last_mouse_y(-1), last_click_time(0), last_draw_event(0)
{

}

Qt::MouseButton 
QtHaikuView::translateMouseButton(uint32 s)
{
    Qt::MouseButton ret = Qt::NoButton;
    if (s & B_PRIMARY_MOUSE_BUTTON)
        ret = Qt::LeftButton;
    if (s & B_SECONDARY_MOUSE_BUTTON)
        ret = Qt::RightButton;
    if (s & B_TERTIARY_MOUSE_BUTTON)
        ret = Qt::MidButton;
    return ret;
}

Qt::MouseButtons 
QtHaikuView::translateMouseButtons(uint32 s)
{
    Qt::MouseButtons ret = 0;
    if (s & B_PRIMARY_MOUSE_BUTTON)
        ret |= Qt::LeftButton;
    if (s & B_SECONDARY_MOUSE_BUTTON)
        ret |= Qt::RightButton;
    if (s & B_TERTIARY_MOUSE_BUTTON)
        ret |= Qt::MidButton;
    return ret;
}

void 
QtHaikuView::sendWheelEvent(int delta_x, int delta_y)
{
	BPoint 		  mouse_pos;
	uint32 		  buttons;
    uint32 		  mod = modifiers();
	
	GetMouse(&mouse_pos,&buttons);
    Qt::KeyboardModifiers qt_mod = translateModifiers(mod);
	Qt::MouseButtons qt_buttons = translateMouseButtons(buttons);
	
	BPoint screen_pos = ConvertToScreen(mouse_pos);
    const QPoint globalPos = QPoint(screen_pos.x,screen_pos.y);
    QPoint pos = QPoint(mouse_pos.x,mouse_pos.y);   
    
	if(delta_x!=0)
		emit sendHaikuEvent(fWidget, new QWheelEvent(pos, globalPos, -delta_x, qt_buttons, qt_mod, Qt::Horizontal));
	if(delta_y!=0)
		emit sendHaikuEvent(fWidget, new QWheelEvent(pos, globalPos, -delta_y, qt_buttons, qt_mod, Qt::Vertical));
}

Qt::KeyboardModifiers 
QtHaikuView::translateModifiers(uint32 s)
{
    Qt::KeyboardModifiers ret = 0;
    if (s & B_SHIFT_KEY)
        ret |= Qt::ShiftModifier;
    if (s & B_CONTROL_KEY)
        ret |= Qt::AltModifier;
    if (s & B_COMMAND_KEY)
        ret |= Qt::ControlModifier;
    return ret;
}

void QtHaikuView::FrameResized(float width, float height)
{
	QSize widgetSize = fWidget->size();
	QSize newSize = QSize(width, height);
	if (widgetSize == newSize)
		return;
	fWidgetPrivate->data.fstrut_dirty = true;

    emit sendUpdateWidget();
}

void
QtHaikuView::Hide()
{
	if (!isVisible) return;
	if (Window() && Window()->LockLooper()){
		BView::Hide();
		isVisible = false;
		Window()->UnlockLooper();
	}
}

void
QtHaikuView::Show()
{
	if (isVisible) return; 
	if (Window() && Window()->LockLooper()){
		BView::Show();
		isVisible = true;
		Window()->UnlockLooper();
	}
}

void
QtHaikuWindow::Hide()
{
	if (LockLooper()){
	if (!IsHidden())
		BWindow::Hide();
		UnlockLooper();
	}
}

void
QtHaikuWindow::Show()
{
	if (LockLooper()){
	if (IsHidden())
		BWindow::Show();
		UnlockLooper();
	}
}


QtHaikuView::~QtHaikuView()
{
//	debugprint(5, "Reimplemented: QtHaikuView::~QtHaikuView\n");
}

inline void QtHaikuView::setWidget(QWidget *widget, QWidgetPrivate  *widgetPrivate)
{
	fWidget = widget;
	fWidgetPrivate = widgetPrivate;
}

void QtHaikuView::MouseUp(BPoint point)
{
	BPoint	point2;
	uint32 	h_mod=modifiers();
	uint32 	h_buttons = 0;
	uint32 	h_button = last_down_buttons;
	last_down_buttons = 0;

	Qt::MouseButton qt_button = translateMouseButton(h_button);
	Qt::MouseButtons qt_buttons = translateMouseButtons(h_buttons);
	Qt::KeyboardModifiers qt_mod = translateModifiers(h_mod);

	BPoint glob_pos = ConvertToScreen(point);
	haiku_global_mouse_x = glob_pos.x;
	haiku_global_mouse_y = glob_pos.y;
	
	QWidget *widget = fWidget->mouseGrabber();
	if(!widget)
		widget = fWidget;
	
	if (qt_button == Qt::RightButton)
		emit sendHaikuEvent(widget, new QContextMenuEvent(QContextMenuEvent::Mouse, 
						QPoint(point.x,point.y), QPoint(haiku_global_mouse_x,haiku_global_mouse_y), qt_mod));
	else		
		emit sendHaikuEvent(widget, new QMouseEvent(QEvent::MouseButtonRelease, 
						QPoint(point.x,point.y), qt_button, qt_buttons, qt_mod));	
}

void QtHaikuView::MouseDown(BPoint point)
{
	uint32 h_mod=modifiers();
	uint32 h_buttons = Window()->CurrentMessage()->FindInt32("buttons");
	uint32 h_button = h_buttons;	//TODO: only one button in per time
	
	Qt::MouseButton qt_button = translateMouseButton(h_button);
	Qt::MouseButtons qt_buttons = translateMouseButtons(h_buttons);
	Qt::KeyboardModifiers qt_mod = translateModifiers(h_mod);
	
	BPoint glob_pos = ConvertToScreen(point);
	haiku_global_mouse_x = glob_pos.x;
	haiku_global_mouse_y = glob_pos.y;
	
	SetMouseEventMask(B_POINTER_EVENTS,B_NO_POINTER_HISTORY);

	QWidget *widget = fWidget->mouseGrabber();
	if(!widget)
		widget = fWidget;		
		
	bigtime_t now=system_time();

	bigtime_t double_click_speed;
	get_click_speed(&double_click_speed);

	if( now - last_click_time < double_click_speed && 
		last_clicked_buttons == h_buttons && 
		last_mouse_x==glob_pos.x && 
		last_mouse_y==glob_pos.y) {
			last_clicked_buttons = 0;
			emit sendHaikuEvent(widget, new QMouseEvent(QEvent::MouseButtonDblClick, 
								QPoint(point.x,point.y), qt_button, qt_buttons, qt_mod));
	} else {
			emit sendHaikuEvent(widget, new QMouseEvent(QEvent::MouseButtonPress, 
								QPoint(point.x,point.y), qt_button, qt_buttons, qt_mod));					
	}
//	qDebug()<<"MouseDown() "<<point.x<<" "<<point.y<<" "<<haiku_global_mouse_x<<" "<<haiku_global_mouse_y;	
	last_click_time=now;
	last_down_buttons = h_buttons;
	last_clicked_buttons = h_buttons;
	last_mouse_x = glob_pos.x;
	last_mouse_y = glob_pos.y;
}

void QtHaikuView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	Q_UNUSED(message);
	if (!fWidget) return;
//	qDebug()<<"MouseMoved() "<<fWidget<<"	"<<this<<"	pos:"<<point.x<<" "<<point.y;
	
	switch(transit)
	{
	 	case B_INSIDE_VIEW:
	 	case B_ENTERED_VIEW:
	 		{
	 			BCursor *curs = fWidget->cursor().handle();	 			
 				if(curs)
	 				SetViewCursor(curs);
	 			else
 					SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
	 		}
	 		break;
	 	case B_EXITED_VIEW:
	 		break;
	}
	
	uint32 	h_mod=modifiers();
	uint32 	h_buttons = last_down_buttons;
	uint32 	h_button = h_buttons;	//TODO: only one button in per time

	Qt::MouseButton qt_button = translateMouseButton(h_button);
	Qt::MouseButtons qt_buttons = translateMouseButtons(h_buttons);
	Qt::KeyboardModifiers qt_mod = translateModifiers(h_mod);
	
	BPoint glob_pos = ConvertToScreen(point);
	
	haiku_global_mouse_x = glob_pos.x;
	haiku_global_mouse_y = glob_pos.y;
	
	QWidget *widget = fWidget->mouseGrabber();
	if(!widget)
		widget = fWidget;
			
	emit sendHaikuEvent(widget, new QMouseEvent(QEvent::MouseMove, QPoint(point.x,point.y), Qt::NoButton, qt_buttons, qt_mod));
//	qDebug()<<"MouseMove()"<<point.x<<" "<<point.y<<" "<<haiku_global_mouse_x<<" "<<haiku_global_mouse_y;	
}

void QtHaikuView::Draw(BRect updateRect)
{
	QRect r = QRect(updateRect.left-1, updateRect.top-1, updateRect.Width()+2,  updateRect.Height()+2);		
	emit sendUpdateWidgetRect(r);
}

void QtHaikuView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
	default:
		BView::MessageReceived(msg);
		break;
	}
}

QtHaikuWindow::QtHaikuWindow(BRect frame, const char *title,  window_look look, window_feel feel, uint32 flags)
		: BWindow(frame, title, look, feel, flags)
{
}

QtHaikuWindow::~QtHaikuWindow()
{
}

void 
QtHaikuWindow::sendKeyEvent(QEvent::Type type, BMessage *msg)
{
    int32 		  key = 0;
    uint32        code = 0;
    uint32 		  mod = modifiers();
	extern 		  QTextCodec *qt_input_mapper; // from qapplication_haiku.cpp
    QChar 		  converted;
	const char	  *bytes;
	QString 	  text;

    if ( msg ) {
      if(msg->FindString("bytes", &bytes) == B_OK) {
	    if (qt_input_mapper)
    	    text = qt_input_mapper->toUnicode(bytes, strlen(bytes), 0);    	
	  }	
      if(msg->FindInt32("key", &key) == B_OK) {
		code = qt_keymapper_private()->translateKeyCode(key);
	  }	
    }
	
    if (text.isEmpty()) {
            text.fromLatin1(bytes);
    }
	
    Qt::KeyboardModifiers qt_mod = fView->translateModifiers(mod);
    
    QWidget *widget;
    widget = QWidget::keyboardGrabber();
    if (!widget) {
        if (QApplicationPrivate::popupWidgets != 0) {
            widget = QApplication::activePopupWidget()->focusWidget();
            if (!widget) {
                widget = QApplication::activePopupWidget();
            }
        } else {
            widget = QApplicationPrivate::focus_widget;
            if (!widget) {
                widget = fView->fWidget;
            }
        }
    }    
    
//    qDebug() << "KeyEvent: " << code << " (" <<text<<")";
        	   
    emit fView->sendHaikuEvent(widget, new QKeyEvent(type, code, qt_mod, text, false, text.length()));
}

void
QtHaikuWindow::DispatchMessage(BMessage *msg, BHandler *handler)
{
	switch(msg->what) {
		case B_UNMAPPED_KEY_DOWN:
		case B_KEY_DOWN:
			{
				sendKeyEvent(QEvent::KeyPress, msg);				
				break;	
			}
		case B_UNMAPPED_KEY_UP:
		case B_KEY_UP:
			{
				sendKeyEvent(QEvent::KeyRelease, msg);				
				break;
			}	
	default:
		break;
	}			
	BWindow::DispatchMessage(msg, handler);
}

void QtHaikuWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what) {
	case B_MOUSE_WHEEL_CHANGED:
		{
			 float shift_x=0;
			 float shift_y=0;
			 if(msg->FindFloat("be:wheel_delta_x",&shift_x)!=B_OK)
			 	shift_x = 0;
			 if(msg->FindFloat("be:wheel_delta_y",&shift_y)!=B_OK)			 	
			 	shift_y = 0;
			 
			 if(shift_x != 0 || shift_y !=0) {
				BView *view = LastMouseMovedView();
				QtHaikuView *widget_view = (QtHaikuView*)view;
				widget_view->sendWheelEvent(shift_x*120, shift_y*120);
			 }
			 break;
		}		
	default:
		BWindow::MessageReceived(msg);
		break;
	}
}

void QtHaikuWindow::FrameResized(float width, float height)
{
	QRect cr = fView->fWidget->geometry();
	
	QSize widgetSize = fView->fWidget->size();
	QSize newSize = QSize(width+1, height+1);
	if (widgetSize == newSize)
		return;

	cr.setSize(newSize);
	fView->fWidgetPrivate->data.crect = cr;
	fView->fWidgetPrivate->data.fstrut_dirty = true;
    emit fView->sendHaikuEvent(fView->fWidget, new QResizeEvent(newSize, widgetSize));
}

void QtHaikuWindow::FrameMoved(BPoint point)
{
	QRect cr = fView->fWidget->geometry();
	QPoint oldPos = cr.topLeft();
	QPoint newCPos(point.x,point.y);
	
	if(oldPos==newCPos)
		return;

	fView->fWidgetPrivate->data.fstrut_dirty = true;
	
	cr.moveTopLeft(newCPos);
	fView->fWidgetPrivate->data.crect = cr;
	
	emit fView->sendHaikuEvent(fView->fWidget, new QMoveEvent(newCPos,oldPos));
}

void QtHaikuWindow::WindowActivated(bool active)
{
	if (active) {
		emit fView->sendSetActiveWindow(fView->fWidget);
	} else {
		while (qApp->activePopupWidget())	// App(window) become inactive - we must close all popup
			qApp->activePopupWidget()->close();
		emit fView->sendSetActiveWindow(0);
	}
}

bool QtHaikuWindow::QuitRequested()
{
	emit fView->sendCloseWindow(fView->fWidget);
	return false;
}

inline void QtHaikuWindow::setView(QtHaikuView *view)
{
	fView = view;
	AddChild(view);

	fView->MoveTo(0,0);
	fView->ResizeTo(Bounds().Width(),Bounds().Height());
	fView->SetResizingMode(B_FOLLOW_ALL_SIDES);	
}

void QtHaikuWindow::updateWindowFlags(Qt::WindowFlags flags)
{
	Qt::WindowType type =  static_cast<Qt::WindowType>(int(flags & Qt::WindowType_Mask)) ;
	
	bool popup = (type == Qt::Popup);
	bool splash = (type == Qt::SplashScreen);
	bool dialog = (type == Qt::Dialog
                   || type == Qt::Sheet
                   || (flags & Qt::MSWindowsFixedSizeDialogHint));
//    bool desktop = (type == Qt::Desktop);
	bool tool = (type == Qt::Tool || type == Qt::Drawer);
	bool tooltip = (type == Qt::ToolTip);

	window_look wlook = B_TITLED_WINDOW_LOOK;
	window_feel wfeel = B_NORMAL_WINDOW_FEEL;
	uint32 wflag = B_NO_WORKSPACE_ACTIVATION | B_NOT_ANCHORED_ON_ACTIVATE ;
	
	if(tool) {
		wlook = B_FLOATING_WINDOW_LOOK;	
		wflag |= B_WILL_ACCEPT_FIRST_CLICK;			
	}
	
	if(splash) {
		wlook = B_NO_BORDER_WINDOW_LOOK;			
	}
	
	if(popup) {
		wlook = B_NO_BORDER_WINDOW_LOOK;			
		wflag |= B_WILL_ACCEPT_FIRST_CLICK|B_AVOID_FRONT|B_AVOID_FOCUS;
		flags |= Qt::WindowStaysOnTopHint;
	}
		
	if (dialog) {
//					if (q->parentWidget())
//						wfeel = B_MODAL_SUBSET_WINDOW_FEEL;
//					else
		//wfeel = B_MODAL_APP_WINDOW_FEEL;								
	}
	
	if (tooltip) {
		wlook = B_NO_BORDER_WINDOW_LOOK;
		wflag |= B_WILL_ACCEPT_FIRST_CLICK|B_AVOID_FOCUS;
	}

    if (flags & Qt::FramelessWindowHint)
    	wlook = B_NO_BORDER_WINDOW_LOOK;
	if (flags & Qt::MSWindowsFixedSizeDialogHint)// (dialog)
    	wflag |= B_NOT_RESIZABLE | B_NOT_ZOOMABLE;               

	if (flags & Qt::CustomizeWindowHint){
		//if (flags & Qt::WindowTitleHint)
			//{ };
		//if (flags & Qt::WindowSystemMenuHint)
			//{ };
		if (!(flags & Qt::WindowMinimizeButtonHint))
    		wflag |= B_NOT_MINIMIZABLE;
		if (!(flags & Qt::WindowMaximizeButtonHint))
			wflag |= B_NOT_ZOOMABLE;					
		if (!(flags & Qt::WindowCloseButtonHint))
			wflag |= B_NOT_CLOSABLE;
		//if (flags & Qt::WindowContextHelpButtonHint)
	//  	exsty |= WS_EX_CONTEXTHELP;
	}

	if (flags & Qt::WindowStaysOnTopHint)
		wfeel = B_FLOATING_ALL_WINDOW_FEEL;	

//	if (flags & Qt::WindowStaysOnBottomHint)
//		wfeel = B_FLOATING_ALL_WINDOW_FEEL;	
	
	SetLook(wlook);
	SetFeel(wfeel);
	SetFlags(wflag);
}

// Qt reimplementation

void QWidgetPrivate::create_sys(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_Q(QWidget);
//    qDebug()<<"create_sys:"<<q<<"	Wid:"<<Wid;
	WId id = 0;
	WId destroyw = 0;
	bool topLevel = (data.window_flags & Qt::Window);
    bool desktop = (q->windowType() == Qt::Desktop);

   	BScreen bscreen(B_MAIN_SCREEN_ID);
	BRect screen_frame  = bscreen.Frame();
   
    if (!window)                                // always initialize
        initializeWindow = true;

    q->setAttribute(Qt::WA_WState_Created, false);
    
    if (desktop){ 
    }

    if (window) {                                // override the old window
        if (destroyOldWindow)
            destroyw = data.winid;
        id = window;
        nativeView = (QtHaikuView *)window;        
       	((QtHaikuWindow *)(window->Window()))->updateWindowFlags(data.window_flags);
    } 
    else 
    if (desktop) {                        // desktop widget
    	if (!q->testAttribute(Qt::WA_DontShowOnScreen)) {
	        //popup = false;                                // force this flags off
			data.crect.setRect(screen_frame.left,screen_frame.top,screen_frame.right,screen_frame.bottom);
    	}
//        id = GetDesktopWindow();
//        setWinId(id);
    } 
    else 
    {
    	if (topLevel) {                       // create top-level widget
		int x=data.crect.left(),
			y=data.crect.top(),
			w=data.crect.width(),
			h=data.crect.height();
			
        const bool wasMoved = q->testAttribute(Qt::WA_Moved);
        const bool wasResized = q->testAttribute(Qt::WA_Resized);
        
        if(!wasResized) {
    		w = screen_frame.right / 2;
	        h = (4 * screen_frame.bottom) / 10;
	        data.crect.setSize(QSize(w, h));
        }
        
        if(!wasMoved) {
        	srand(time(0));
        	int dx = (screen_frame.right - w)/4;
        	int dy = (screen_frame.bottom - h)/4;        	
        	x = dx==0?0:rand() % dx + (dx/2);
        	y = dy==0?0:rand() % dy + (dy/2);
        	data.crect.moveTo(x,y);
        }        			
  			
		QString title = q->windowTitle();
		if (title.isEmpty())
			title = QFileInfo(QApplication::applicationFilePath()).fileName();
		nativeWindow = new QtHaikuWindow(BRect(x,y,x+w-1,y+h-1), title.toUtf8(), B_NO_BORDER_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0);
		nativeWindow->updateWindowFlags(data.window_flags);
		nativeView = new QtHaikuView(BRect(0,0,w,h), "RootView",  B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
		nativeWindow->setView(nativeView);
    } else {
		nativeView = new QtHaikuView(BRect(data.crect.x(),data.crect.y(),data.crect.x()+data.crect.width(),data.crect.y()+data.crect.height()),
							   "WidgetView",  B_FOLLOW_NONE, B_WILL_DRAW/* | B_NAVIGABLE*/);
	    q->parentWidget()->d_func()->nativeView->AddChild(nativeView);
		nativeWindow = q->parentWidget()->d_func()->nativeWindow;
	}

	nativeView->setWidget(q, this);
	if (nativeView->LockLooper()){
		nativeView->SetViewColor(B_TRANSPARENT_32_BIT);
		nativeView->UnlockLooper();
	}
	if (q->isWindow()) { }//nativeWindow->Hide(); // BWindow created hidden (?)
	else 
		nativeView->Hide();
    }

	setWinId(nativeView);	
    
    if (desktop) {
        q->setAttribute(Qt::WA_WState_Visible);
    } else if (topLevel && !q->testAttribute(Qt::WA_DontShowOnScreen)) {
/*        RECT  cr;
        GetClientRect(id, &cr);
        // one cannot trust cr.left and cr.top, use a correction POINT instead
        POINT pt;
        pt.x = 0;
        pt.y = 0;
        ClientToScreen(id, &pt);

        if (data.crect.width() == 0 || data.crect.height() == 0) {
            data.crect = QRect(pt.x, pt.y, data.crect.width(), data.crect.height());
        } else {
            data.crect = QRect(QPoint(pt.x, pt.y),
                               QPoint(pt.x + cr.right - 1, pt.y + cr.bottom - 1));
        }

        if (data.fstrut_dirty) {
            // be nice to activeqt
            updateFrameStrut();
        }*/
    }

		
    q->setAttribute(Qt::WA_WState_Created);                // accept move/resize events
    q->setAttribute(Qt::WA_WState_Visible, false);   

    if (window) {                                // got window from outside
//        if (IsWindowVisible(window))
//            q->setAttribute(Qt::WA_WState_Visible);
//        else
//            q->setAttribute(Qt::WA_WState_Visible, false);
    }

    if (extra && !extra->mask.isEmpty())
        setMask_sys(extra->mask);

    if (q->hasFocus() && q->testAttribute(Qt::WA_InputMethodEnabled)) {
		q->setFocus();		
    }
 
    //if (destroyw) {
        //BWindow *win = destroyw->Window();
        //if(win->Lock()) {
        //	win->Quit();
        //	win->Unlock();
        //}
    //}

//    if (q != qt_tablet_widget && QWidgetPrivate::mapper)
//        qt_tablet_init();

    if (q->testAttribute(Qt::WA_DropSiteRegistered))
        registerDropSite(true);

//    if (maybeTopData() && maybeTopData()->opacity != 255)
//        q->setWindowOpacity(maybeTopData()->opacity/255.);

    if (topLevel && (data.crect.width() == 0 || data.crect.height() == 0)) {
        q->setAttribute(Qt::WA_OutsideWSRange, true);
    }

    if (!topLevel && q->testAttribute(Qt::WA_NativeWindow) && q->testAttribute(Qt::WA_Mapped)) {
		nativeView->Window()->Show();
    }
    
//   qDebug() << "=< QWidgetPrivate::create_sys();  QWidget = "<< q ;
}

void QWidgetPrivate::hide_sys()
{
	Q_Q(QWidget);
    deactivateWidgetCleanup();
    Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

    if (q->isWindow())
    	nativeWindow->Hide();
    else
    	nativeView->Hide();
    q->setAttribute(Qt::WA_Mapped, false);
}

void QWidgetPrivate::show_sys()
{
	Q_Q(QWidget);
	Q_ASSERT(q->testAttribute(Qt::WA_WState_Created));

	if (q->testAttribute(Qt::WA_OutsideWSRange)){
		qDebug("Qt::WA_OutsideWSRange");
		return;
	}
	
	if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
		qDebug("Qt::WA_DontShowOnScreen");
		invalidateBuffer(q->rect());
		return;
	}

	//hack for native resize holder for window
	if(q->inherits("QSizeGrip")) {
		nativeView->Window()->SetLook(B_DOCUMENT_WINDOW_LOOK);
		q->setAttribute(Qt::WA_WState_Visible, false);
		return;
	}

	if (q->isWindow()) {
		if (q->isMinimized()) {
			// ### TODO: Should handle
		} else if (q->isMaximized()) {
			// ### TODO: Should handle
		}
	}
	if (q->testAttribute(Qt::WA_ShowWithoutActivating)
		|| (q->windowType() == Qt::Popup)
		|| (q->windowType() == Qt::ToolTip)
		|| (q->windowType() == Qt::Tool)) {
		// ### TODO: Should Handle
	}

//	if (data.fstrut_dirty)
//		updateFrameStrut();
    if (q->isWindow())
    	nativeWindow->Show();
    else
    	nativeView->Show();

	q->setAttribute(Qt::WA_Mapped);
			
	invalidateBuffer(q->rect());
}

void QWidgetPrivate::setParent_sys(QWidget* parent, Qt::WindowFlags f)
{
	Q_Q(QWidget);
	
    bool wasCreated = q->testAttribute(Qt::WA_WState_Created);
    if (q->isVisible() && q->parentWidget() && parent != q->parentWidget())
        q->parentWidget()->d_func()->invalidateBuffer(q->geometry());

    WId old_winid = data.winid;
	bool old_w = q->isWindow();
    bool widgetTypeChanged = false;

    if (wasCreated && q->isVisible()) {
		if (q->isWindow())
			nativeWindow->Hide();
			else
			nativeView->Hide();
    }

    bool dropSiteWasRegistered = false;
    if (q->testAttribute(Qt::WA_DropSiteRegistered)) {
        dropSiteWasRegistered = true;
        q->setAttribute(Qt::WA_DropSiteRegistered, false); // ole dnd unregister (we will register again below)
    }

    if ((q->windowType() == Qt::Desktop))
        old_winid = 0;

    QObjectPrivate::setParent_helper(parent);

    bool explicitlyHidden = q->testAttribute(Qt::WA_WState_Hidden) && q->testAttribute(Qt::WA_WState_ExplicitShowHide);

    data.window_flags = f;
    data.fstrut_dirty = true;
    q->setAttribute(Qt::WA_WState_Created, false);
    q->setAttribute(Qt::WA_WState_Visible, false);
    q->setAttribute(Qt::WA_WState_Hidden, false);
    adjustFlags(data.window_flags, q);
	widgetTypeChanged = old_w != q->isWindow(); // widget type changed (widget<->window)

	if (wasCreated){
		if (parent && !parent->testAttribute(Qt::WA_WState_Created))
			parent->createWinId();

		if (widgetTypeChanged){
			
			BWindow *win = nativeView->Window();
			if (win && win->LockLooper()){
				nativeView->RemoveSelf();
				if (old_w)
					win->Quit();
				else
					win->UnlockLooper();
			}

			if (old_w){ //was Window, become Widget(non-Window)
				bool wasLocked = parent->nativeView()->Looper()->IsLocked();
				if (wasLocked || parent->nativeView()->LockLooper() )
				{
					parent->nativeView()->AddChild(nativeView);
					nativeView->Hide();
	
					nativeView->SetResizingMode(B_FOLLOW_NONE);
					nativeView->MoveTo(data.crect.x(),data.crect.y());
					nativeView->ResizeTo(data.crect.x()+data.crect.width(),data.crect.y()+data.crect.height());
					if (!wasLocked)
						parent->nativeView()->UnlockLooper();
				}
				nativeWindow = (QtHaikuWindow *)parent->nativeView()->Window();

				q->setAttribute(Qt::WA_WState_Created, true);
			}
			else{// become Window (topLevel)
				createWinId();
			
				nativeView->RemoveSelf(); //Delete new view created in create_sys. We need only BWindow
				delete nativeView;
				
				nativeView = (QtHaikuView *)old_winid;
				nativeWindow->setView(nativeView);

				nativeView->Show();					
				setWinId(nativeView);
			}
		}
		else // widgetTypeChanged == false
		{
		if (q->isWindow())
			nativeWindow->updateWindowFlags(data.window_flags);
		else{//already created widget change parent widget from one to another
				BWindow *win = nativeView->Window();
				if (win && win->LockLooper()){
					nativeView->RemoveSelf();
					win->UnlockLooper();
				}

				if (parent->nativeView()->LockLooper()){
					parent->nativeView()->AddChild(nativeView);
					parent->nativeView()->UnlockLooper();
					nativeWindow = (QtHaikuWindow *)parent->nativeView()->Window();
				}
		}
		q->setAttribute(Qt::WA_WState_Created, true);
		}
	}
	else
		if (!q->isWindow() && parent->testAttribute(Qt::WA_WState_Created))
			createWinId();

    if (q->isWindow() || (!parent || parent->isVisible()) || explicitlyHidden)
        q->setAttribute(Qt::WA_WState_Hidden);
    q->setAttribute(Qt::WA_WState_ExplicitShowHide, explicitlyHidden);

    if (extra && !extra->mask.isEmpty()) {
        QRegion r = extra->mask;
        extra->mask = QRegion();
        q->setMask(r);
    }
    if (extra && extra->topextra && !extra->topextra->caption.isEmpty()) {
        //setWindowIcon_sys(true);
        setWindowTitle_helper(extra->topextra->caption);
    }

    if (q->testAttribute(Qt::WA_AcceptDrops) || dropSiteWasRegistered
        || (!q->isWindow() && q->parentWidget() && q->parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)))
        q->setAttribute(Qt::WA_DropSiteRegistered, true);

    invalidateBuffer(q->rect());
}

void QWidgetPrivate::raise_sys()
{
//	Q_Q(QWidget);
//	qDebug() << "Unimplemented: QWidgetPrivate::raise_sys() "<<  q ;
}

void QWidgetPrivate::lower_sys()
{
//	Q_Q(QWidget);
//	qDebug() << "Unimplemented: QWidgetPrivate::lower_sys() "<<  q ;
}

void QWidgetPrivate::stackUnder_sys(QWidget* widget)
{
//	Q_Q(QWidget);
//	qDebug() << "Unimplemented: QWidgetPrivate::stackUnder_sys() "<<  q << "	top:"<<widget;
}

void QWidgetPrivate::setWindowIcon_sys(bool)
{
//	Q_Q(QWidget);
//	qDebug() << "Unimplemented: QWidgetPrivate::setWindowIcon_sys() "<<  q ;
}

void QWidgetPrivate::setWindowTitle_sys(const QString &title)
{
	if (q_func()->isWindow() && nativeWindow)
		nativeWindow->SetTitle(title.toUtf8());
}

void QWidgetPrivate::updateSystemBackground()
{
//	Q_Q(QWidget);
//	qDebug() << "Unimplemented: QWidgetPrivate::UpdateSystemBackground() "<<  q ;
}

void QWidgetPrivate::setModal_sys()
{
//	Q_Q(QWidget);
//	qDebug() << "Unimplemented: QWidgetPrivate::setModal_sys() "<<  q ;
}

void QWidgetPrivate::setConstraints_sys()
{
	Q_Q(QWidget);
//	qDebug() << "QWidgetPrivate::setConstraints_sys() "<<  q ;
	if (!q->isWindow() || !q->testAttribute(Qt::WA_WState_Created) || !extra)
		return;
	float minw = 0, minh = 0;
	float maxw = 10000, maxh = 10000;
	if (extra->minw > 0 || extra->minh > 0) {
    	// add minimum size hints
		minw  = qMin(10000, extra->minw);
		minh = qMin(10000, extra->minh);
	}
	if (extra->maxw < QWIDGETSIZE_MAX || extra->maxh < QWIDGETSIZE_MAX) {
		// add maximum size hints
		maxw = qMin(10000, extra->maxw);
		maxh = qMin(10000, extra->maxh);
	}
	nativeWindow->SetSizeLimits(minw,maxw,minh,maxh);
}

void QWidgetPrivate::setCursor_sys(const QCursor& cursor)
{
       Q_UNUSED(cursor);
}

void QWidgetPrivate::unsetCursor_sys()
{
}

void QWidgetPrivate::setWSGeometry(bool dontShow, const QRect &)
{
	Q_UNUSED(dontShow);
	Q_Q(QWidget);
	qDebug() << "Unimplemented: QWidgetPrivate::setWSGeomerty() "<<  q ;
}

void QWidgetPrivate::setGeometry_sys(int x,int y,int w,int h,bool  isMove)
{
	Q_Q(QWidget);
//	qDebug()<<"setGeometry_sys() "<<q<<" "<<x<<" "<<y<<" "<<w<<" "<<h<<" "<<isMove;    		
	BRect nativeFrame;
	
	if (q->isWindow())
	{
		nativeFrame = nativeView->Window()->Frame();
	}
	else
	{ 
		if(nativeView->LockLooper()) {
			nativeFrame = nativeView->Frame();
			nativeView->UnlockLooper();
		}
	}
	
	QPoint oldPos(nativeFrame.left, nativeFrame.top);
	QPoint newPos(x,y);
	QSize oldSize(nativeFrame.IntegerWidth(), nativeFrame.IntegerHeight());
	QSize newSize(w,h);

	isMove = oldPos != newPos;
	bool isResize = oldSize != newSize;
	
	if (!isResize && !isMove)
		return;

	data.crect.setRect(x, y, w, h);
	
    QTLWExtra *tlwExtra = q->window()->d_func()->maybeTopData();
    const bool inTopLevelResize = tlwExtra ? tlwExtra->inTopLevelResize : false;
		
	if (q->isWindow()){
        if (w == 0 || h == 0) {
            q->setAttribute(Qt::WA_OutsideWSRange, true);
            if (q->isVisible() && q->testAttribute(Qt::WA_Mapped))
                hide_sys();
        } else if (q->isVisible() && q->testAttribute(Qt::WA_OutsideWSRange)) {
            q->setAttribute(Qt::WA_OutsideWSRange, false);

            // put the window in its place and show it
            if (data.winid){
            	nativeView->Window()->MoveTo(x, y);
				nativeView->Window()->ResizeTo(w-1, h-1);
            }	
			setConstraints_sys();//do_size_hints(q, extra);
            show_sys();
        } else {
			if (isMove){	
				nativeView->Window()->MoveTo(x, y);
      		}
			if (isResize){	
				nativeView->Window()->ResizeTo(w-1, h-1);
			}
        }
		updateFrameStrut();
	} else {	//not Window	
		if (isResize) {
			if(nativeView->LockLooper()) {
				nativeView->ResizeTo(w, h);
				nativeView->UnlockLooper();
			}
		}
		if (isMove)
		{
			if(nativeView->LockLooper()) {
				nativeView->MoveTo(x, y);
				nativeView->UnlockLooper();
			}
		}
	}

    if (q->isWindow() && q->isVisible() && isResize && !inTopLevelResize) {
        invalidateBuffer(q->rect()); //after the resize
    }
	
	if (q->isVisible()) {
        if (isMove && q->pos() != oldPos) {
            QMoveEvent e(q->pos(), oldPos);
            QApplication::sendEvent(q, &e);
        }
        if (isResize) {
//            static bool slowResize = qgetenv("QT_SLOW_TOPLEVEL_RESIZE").toInt();
            // If we have a backing store with static contents, we have to disable the top-level
            // resize optimization in order to get invalidated regions for resized widgets.
            // The optimization discards all invalidateBuffer() calls since we're going to
            // repaint everything anyways, but that's not the case with static contents.
//            const bool setTopLevelResize = !slowResize && q->isWindow() && extra && extra->topextra
//                                           && !extra->topextra->inTopLevelResize
//                                           && (!extra->topextra->backingStore
//                                               || !extra->topextra->backingStore->hasStaticContents());
//            if (setTopLevelResize)
//                extra->topextra->inTopLevelResize = true;
            QResizeEvent e(q->size(), oldSize);
            QApplication::sendEvent(q, &e);
//            if (setTopLevelResize)
  //              extra->topextra->inTopLevelResize = false;
        }
    } else {
        if (isMove && q->pos() != oldPos)
            q->setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            q->setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}

void QWidgetPrivate::registerDropSite(bool)
{
	//debugprint(1, "Unimplemented: QWidgetPrivate::registerDropSite\n");
}

void QWidgetPrivate::setWindowOpacity_sys(qreal)
{
	//debugprint(1, "Unimplemented: QWidgetPrivate::setWindowOpacity_sys \n");
}
/*
void QWidgetPrivate::createTLExtra()
{
//	fprintf(stderr, "Unimplemented: QWidgetPrivate::createTLExtra\n");
}
*/
void QWidgetPrivate::deleteSysExtra()
{
//	debugprint(1, "Unimplemented: QWidgetPrivate::deleteSysExtra\n");
}

void QWidgetPrivate::deleteTLSysExtra()
{
	//debugprint(1, "Unimplemented: QWidgetPrivate::deleteTLSysExtra\n");
}

void QWidgetPrivate::createSysExtra()
{
//	debugprint(1, "Unimplemented: QWidgetPrivate::createSysExtra\n");
}

void QWidgetPrivate::createTLSysExtra()
{
//	debugprint(1, "Unimplemented: QWidgetPrivate::createTLSysExtra\n");
}

void QWidgetPrivate::setWindowIconText_sys(const QString&)
{
//	debugprint(1, "Unimplemented: QWidgetPrivate::setWindowIconText_sys \n");
}

void QWidgetPrivate::scroll_sys(int,int, const QRect& r)
{
//	debugprint(1, "Reimplemented: QWidgetPrivate::scroll(int,int,const  QRect&\n");
	Q_Q(QWidget);
	q->update(r);
}

void QWidgetPrivate::scroll_sys(int, int)
{
//	debugprint(1, "Reimplemented: QWidgetPrivate::scroll(int,int)\n");
	Q_Q(QWidget);
	q->update();
}

void QWidgetPrivate::updateFrameStrut()
{
//	debugprint(5, "Reimplemented: QWidgetPrivate::updateFrameStrut\n");
	QTLWExtra* top = this->topData();
	if(nativeView->LockLooper()) {
		BRect frame = nativeView->Bounds();
		top->frameStrut.setCoords(frame.left, frame.top, frame.right,  frame.bottom);
		nativeView->UnlockLooper();
	}
	data.fstrut_dirty = false;
}

void QWidgetPrivate::setFocus_sys()
{
}

QWindowSurface *QWidgetPrivate::createDefaultWindowSurface_sys()
{
	Q_Q(QWidget);
	return new QRasterWindowSurface(q);
}

void QWidgetPrivate::setMask_sys(const QRegion &region)
{
	Q_UNUSED(region);
//	debugprint(1, "Unimplemented: QWidgetPrivate::setMask_sys()\n");
}

int QWidget::metric(PaintDeviceMetric m) const
{
//qDebug() << "QWidget::metric("<< m <<")";
	int val = 0;
	if (m == PdmWidth) {
		val = data->crect.width();
	} else if (m == PdmHeight) {
		val = data->crect.height();
	} else {
		switch (m) {
		case PdmDpiX:
		case PdmPhysicalDpiX:
		case PdmDpiY:
		case PdmPhysicalDpiY:
			val = 72;
			break;
		case PdmWidthMM:
		case PdmHeightMM:
		case PdmNumColors:
		case PdmDepth:
//			qWarning("QWidget::metric Unknown metric type requested\n");
			break;
		default:
			val = 0;
			qWarning("QWidget::metric: Invalid metric command");
		}
	}

	return val;
}

QPaintEngine *QWidget::paintEngine() const
{
	return 0;
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
	//Q_D(QWidget);
//	qDebug()<<"QWidget::mapToGlobal() pos:"<<pos;
    if (!testAttribute(Qt::WA_WState_Created) || !internalWinId()) {
        QPoint p = pos + data->crect.topLeft();
        //cannot trust that !isWindow() implies parentWidget() before create
        return (isWindow() || !parentWidget()) ?  p : parentWidget()->mapToGlobal(p);
    }

	if (nativeView()->LockLooper())
	{
		BPoint p = nativeView()->ConvertToScreen(BPoint(pos.x(),pos.y()));
		nativeView()->UnlockLooper();
		return QPoint(p.x,p.y);
	}
	return QPoint(); // ???
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
//		qDebug()<<"QWidget::mapFromGlobal() pos:"<<pos;
    if (!testAttribute(Qt::WA_WState_Created) || !internalWinId()) {
        //cannot trust that !isWindow() implies parentWidget() before create
        QPoint p = (isWindow() || !parentWidget()) ?  pos : parentWidget()->mapFromGlobal(pos);
        return p - data->crect.topLeft();
    }

	if (nativeView()->LockLooper())
	{
		BPoint p = nativeView()->ConvertFromScreen(BPoint(pos.x(),pos.y()));
		nativeView()->UnlockLooper();
		return QPoint(p.x,p.y);
	}
	return QPoint(); // ???
}

void QWidget::activateWindow()
{
	Q_D(QWidget);
    if (isWindow()) {
       d->nativeView->Window()->Activate();
    }   
}

void QWidget::setWindowState(Qt::WindowStates newstate)
{
	qDebug() << "QWidgetPrivate::setWindowState() "<<  this<<"	"<<newstate ;

	Q_D(QWidget);
	Qt::WindowStates oldstate = windowState();
	if (oldstate == newstate)
		return;

    if (isWindow()) {
        createWinId();
//        Q_ASSERT(testAttribute(Qt::WA_WState_Created));

        // Ensure the initial size is valid, since we store it as normalGeometry below.
        if (!testAttribute(Qt::WA_Resized) && !isVisible())
            adjustSize();
		// Qt::WindowMaximized
        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if (newstate & Qt::WindowMaximized && !(oldstate & Qt::WindowFullScreen))
                d->topData()->normalGeometry = geometry();
            if (isVisible() && !(newstate & Qt::WindowMinimized)) {
//TODO: Qt::WindowMaximized not supported yet.
				d->nativeView->Window()->Show();
								
                if (!(newstate & Qt::WindowFullScreen)) {
                    QRect r = d->topData()->normalGeometry;
                    if (!(newstate & Qt::WindowMaximized) && r.width() >= 0) {
                        if (pos() != r.topLeft() || size() !=r.size()) {
                            d->topData()->normalGeometry = QRect(0,0,-1,-1);
                            setGeometry(r);
                        }
                    }
                } else {
                    d->updateFrameStrut();
                }
            }
        }

		// Qt::WindowFullScreen
        if ((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if (newstate & Qt::WindowFullScreen) { // save current window flags and geometry and go to FullScreen
                if (d->topData()->normalGeometry.width() < 0 && !(oldstate & Qt::WindowMaximized))
                    d->topData()->normalGeometry = geometry();
                d->topData()->savedFlags = d->data.window_flags;


				d->data.window_flags |= Qt::SplashScreen;				
				d->nativeWindow->updateWindowFlags(d->data.window_flags);

    	    	QRect r = qApp->desktop()->screenGeometry(this);
				d->nativeView->Window()->Zoom(BPoint(r.left(),r.top()),r.width(),r.height());
				d->updateFrameStrut();
            }
            else { //back from FullScreen (to saved flags and geomerty)
				d->data.window_flags = d->topData()->savedFlags;
				d->nativeWindow->updateWindowFlags(d->data.window_flags);

                if (!(newstate & Qt::WindowMaximized)) {
                    QRect r = d->topData()->normalGeometry;
                    d->topData()->normalGeometry = QRect(0,0,-1,-1);
                    if (r.isValid())
                        setGeometry(r);
                }
                d->updateFrameStrut();				
            }
            }
        }
        
		//Qt::WindowMinimized
        if ((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized)) {
            if (isVisible())
//TODO: Qt::WindowMinimized state changed...
				d->nativeView->Window()->Minimize((newstate & Qt::WindowMinimized) ? true :
                                    (newstate & Qt::WindowMaximized) ? false : false);
    }

	data->window_state = newstate;
	QWindowStateChangeEvent e(oldstate);
	QApplication::sendEvent(this, &e);
}

void QWidget::grabMouse()
{
   if (mouseGrb)
        mouseGrb->releaseMouse();
   mouseGrb = this;
}

void QWidget::grabMouse(const QCursor &cursor)
{
	Q_UNUSED(cursor);
	if (mouseGrb)
		mouseGrb->releaseMouse();
	mouseGrb = this;
}

void QWidget::releaseMouse()
{
   if(mouseGrb == this) {
        mouseGrb = 0;
   }
}

QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

void QWidget::grabKeyboard()
{
    if (keyboardGrb)
        keyboardGrb->releaseKeyboard();
    keyboardGrb = this;	
}

void QWidget::releaseKeyboard()
{
    if (keyboardGrb == this) {
        keyboardGrb = 0;
    }	
}

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    Q_D(QWidget);
//	qDebug() << "Reimplemented: QWidgetPrivate::destroy() "<<  this ;	    
    if (!isWindow() && parentWidget())
        parentWidget()->d_func()->invalidateBuffer(geometry());
    d->deactivateWidgetCleanup();
    if (testAttribute(Qt::WA_WState_Created)) {
        setAttribute(Qt::WA_WState_Created, false);
        QObjectList childList = children();
        for (int i = 0; i < childList.size(); ++i) { // destroy all widget children
            register QObject *obj = childList.at(i);
            if (obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows,
                                                    destroySubWindows);
        }

        if (mouseGrb == this)
            releaseMouse();
            
        if (keyboardGrb == this)
            releaseKeyboard();

//        if (testAttribute(Qt::WA_ShowModal))                // just be sure we leave modal
//            QApplicationPrivate::leaveModal(this);
//        else 
		if ((windowType() == Qt::Popup))
            qApp->d_func()->closePopup(this);
        
        if (isWindow()) {
            if ((windowType() != Qt::Desktop) && destroyWindow){
                BWindow *win = d->nativeView->Window();
					
				if (win->LockLooper()){
					for (int i =0;i<=win->CountChildren()-1;i++){
						BView *w = win->ChildAt(i);
						w->RemoveSelf();
						delete w;
					}
					win->Quit();
				}
			}
	    } else {
	    	BWindow *win =d->nativeView->Window();
			bool wasLocked = win->Looper()->IsLocked();
			if (win && (wasLocked || win->LockLooper()) ){
	    		d->nativeView->RemoveSelf();
	    		if (!wasLocked)
	    			win->UnlockLooper();
			}
	    	delete d->nativeView;
	    }

		d->nativeView = NULL;
		d->nativeWindow = NULL;
	    d->setWinId(0);     
    }
}

BView* QWidget::nativeView() const
{
	Q_D(const QWidget);
	return d->nativeView;
}
