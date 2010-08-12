#include "qapplication_p.h"
#include "qsessionmanager.h"
#include "qapplication.h"
#include "qevent.h"
#include "qeventdispatcher_haiku_p.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "private/qsystemtrayicon_p.h"
#include <QtGui>
#include <Application.h>
#include <Path.h>
#include <Entry.h>
#include <String.h>
#include <OS.h>
#include <QDebug>
#include <stdio.h>

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static WId curWin;

// ignore the next release event if return from a modal widget
Q_GUI_EXPORT bool qt_win_ignoreNextMouseReleaseEvent = false;

#if defined(QT_DEBUG)
static bool        appNoGrab        = false;        // mouse/keyboard grabbing
#endif

static bool        app_do_modal           = false;        // modal mode
extern QWidgetList *qt_modal_stack;
//extern QDesktopWidget *qt_desktopWidget;
//static QPointer<QWidget> popupButtonFocus;

QPointer<QWidget> qt_last_mouse_receiver = 0;
QWidget* qt_button_down = 0;
QTextCodec * qt_input_mapper = 0;

char *qAppSignature = NULL;

bool qt_nograb()                                // application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return false;
#endif
}

class HQApplication : public BApplication
{
public:
	HQApplication(const char*, QApplicationPrivate *priv);
	~HQApplication();

	virtual void MessageReceived(BMessage *message);
	void	ArgvReceived(int32 argc, char **argv);
	void	RefsReceived(BMessage *pmsg);
	bool	RefHandled;
	entry_ref Ref;
private:
	BMessenger  fTrackerMessenger;
	QApplicationPrivate *priv_ptr;
};

namespace {
static HQApplication* happ = 0;
}

HQApplication::HQApplication(const char* signature, QApplicationPrivate *priv)
		: BApplication(signature)
{
	RefHandled = false;
	
	qDebug("Reimp: HQApplication::HQApplication\n");

	if(qAppSignature)
		free(qAppSignature);
	qAppSignature = strdup(signature);
	
	priv_ptr = priv;	
}

HQApplication::~HQApplication()
{
	qDebug("Reimp: HQApplication::~HQApplication\n");
}

void HQApplication::MessageReceived(BMessage* msg)
{
	BApplication::MessageReceived(msg);
}

void 
HQApplication::RefsReceived(BMessage *pmsg)
{
	if (pmsg->HasMessenger("TrackerViewToken")) {
		pmsg->FindMessenger("TrackerViewToken", &fTrackerMessenger);
	}

	uint32 type;
	int32 count;
	status_t ret = pmsg->GetInfo("refs", &type, &count);
	if (ret != B_OK || type != B_REF_TYPE)
		return;

	entry_ref ref;
	for (int32 i = 0; i < count; i++) {
   		if (pmsg->FindRef("refs", i, &ref) == B_OK) {
   			BPath p(&ref);
   			Ref = ref;
   			RefHandled = true;
   		}
   	}
}

void
HQApplication::ArgvReceived(int32 argc, char **argv)
{
}


int32 AppThread(void *data)
{
	HQApplication *app = (HQApplication*)data;
	app->LockLooper();
	app->Run();	
}


void qt_init(QApplicationPrivate *priv, int)
{
	int i;
	qDebug("qt_init()");

	thread_id my_thread;	

	happ = new HQApplication("application/x-vnd."+QFileInfo(QApplication::applicationFilePath()).fileName().toLatin1(), priv );	
	be_app = happ;
    		
	QApplicationPrivate::haiku_apply_settings();	
	
	my_thread = spawn_thread(AppThread,"app_thread",1,(void*)happ);
	resume_thread(my_thread);
		
	happ->UnlockLooper();

	if(priv->argc==1) {
		for(i=0;i<100;i++) {
			if(happ->RefHandled) {
				BPath p(&happ->Ref);
				priv->argc = 2;
				priv->argv[1]=strdup(p.Path());
				priv->argv[2]=0;
				break;
			}
			snooze(1000);
		}	
		
		QString appDir = QCoreApplication::applicationDirPath();
		chdir(appDir.toUtf8());		
	}
}

void qt_cleanup()
{
	qDebug("qt_cleanup()");
	if (happ)
	{
		happ->Lock();
		happ->Quit();
	}
}

void QApplicationPrivate::_q_alertTimeOut()
{
	qDebug("Unimplemented: void  QApplicationPrivate::_q_alertTimeOut()\n");
}

QString QApplicationPrivate::appName() const
{
	return QCoreApplicationPrivate::appName();
}

void QApplicationPrivate::createEventDispatcher()
{
	Q_Q(QApplication);
	qDebug("Reimplemented: void  QApplicationPrivate::createEventDispatcher\n");
	if (q->type() != QApplication::Tty)
		eventDispatcher = new QEventDispatcherHaiku(q);
	else
		eventDispatcher = new QEventDispatcherUNIX(q);
}

/*****************************************************************************
  Popup widget mechanism

  openPopup()
        Adds a widget to the list of popup widgets
        Arguments:
            QWidget *widget        The popup widget to be added

  closePopup()
        Removes a widget from the list of popup widgets
        Arguments:
            QWidget *widget        The popup widget to be removed
 *****************************************************************************/


void QApplicationPrivate::openPopup(QWidget *popup)
{
    if (!QApplicationPrivate::popupWidgets)
        QApplicationPrivate::popupWidgets = new QWidgetList;
    QApplicationPrivate::popupWidgets->append(popup);
    if (!popup->isEnabled())
        return;

    if (QApplicationPrivate::popupWidgets->count() == 1 && !qt_nograb()) {
        Q_ASSERT(popup->testAttribute(Qt::WA_WState_Created));
//        setAutoCapture(popup->internalWinId());        // grab mouse/keyboard
    }
    // Popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    if (popup->focusWidget()) {
        popup->focusWidget()->setFocus(Qt::PopupFocusReason);
    } else if (QApplicationPrivate::popupWidgets->count() == 1) { // this was the first popup
        if (QWidget *fw = q_func()->focusWidget()) {
            QFocusEvent e(QEvent::FocusOut, Qt::PopupFocusReason);
            q_func()->sendEvent(fw, &e);
        }
    }
}

void QApplicationPrivate::closePopup(QWidget *popup)
{
    if (!QApplicationPrivate::popupWidgets)
        return;
    QApplicationPrivate::popupWidgets->removeAll(popup);
//    POINT curPos;
//    GetCursorPos(&curPos);

    if (QApplicationPrivate::popupWidgets->isEmpty()) { // this was the last popup
        delete QApplicationPrivate::popupWidgets;
        QApplicationPrivate::popupWidgets = 0;
//        replayPopupMouseEvent = (!popup->geometry().contains(QPoint(curPos.x, curPos.y))
//                                && !popup->testAttribute(Qt::WA_NoMouseReplay));
        if (!popup->isEnabled())
            return;
//        if (!qt_nograb())                        // grabbing not disabled
//            releaseAutoCapture();
        QWidget *fw = QApplicationPrivate::active_window ? QApplicationPrivate::active_window->focusWidget()
            : q_func()->focusWidget();
        if (fw) {
            if (fw != q_func()->focusWidget()) {
                fw->setFocus(Qt::PopupFocusReason);
            } else {
                QFocusEvent e(QEvent::FocusIn, Qt::PopupFocusReason);
                q_func()->sendEvent(fw, &e);
            }
        }
    } else {
        // Popups are not focus-handled by the window system (the
        // first popup grabbed the keyboard), so we have to do that
        // manually: A popup was closed, so the previous popup gets
        // the focus.
        QWidget* aw = QApplicationPrivate::popupWidgets->last();
        if (QApplicationPrivate::popupWidgets->count() == 1) {
            Q_ASSERT(aw->testAttribute(Qt::WA_WState_Created));
//            setAutoCapture(aw->internalWinId());
        }
        if (QWidget *fw = aw->focusWidget())
            fw->setFocus(Qt::PopupFocusReason);
    }
}

void QApplicationPrivate::initializeWidgetPaletteHash()
{
	qDebug("Unimplemented:  QApplicationPrivate::initializeWidgetPaletteHash\n");
}

/*! \internal
    apply the settings to the application
*/
bool QApplicationPrivate::haiku_apply_settings()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));

    settings.beginGroup(QLatin1String("Qt"));

    /*
      Qt settings. This is now they are written into the datastream.

      Palette / *                - QPalette
      font                       - QFont
      libraryPath                - QStringList
      style                      - QString
      doubleClickInterval        - int
      keyboardInputInterval  - int
      cursorFlashTime            - int
      wheelScrollLines           - int
      colorSpec                  - QString
      defaultCodec               - QString
      globalStrut/width          - int
      globalStrut/height         - int
      GUIEffects                 - QStringList
      Font Substitutions/ *      - QStringList
      Font Substitutions/...     - QStringList
    */

    QStringList strlist;
    int i;
    QPalette pal(Qt::black);
    int groupCount = 0;
    strlist = settings.value(QLatin1String("Palette/active")).toStringList();
    if (!strlist.isEmpty()) {
        ++groupCount;
        for (i = 0; i < qMin(strlist.count(), int(QPalette::NColorRoles)); i++)
            pal.setColor(QPalette::Active, (QPalette::ColorRole) i,
                         QColor(strlist[i]));
    }
    strlist = settings.value(QLatin1String("Palette/inactive")).toStringList();
    if (!strlist.isEmpty()) {
        ++groupCount;
        for (i = 0; i < qMin(strlist.count(), int(QPalette::NColorRoles)); i++)
            pal.setColor(QPalette::Inactive, (QPalette::ColorRole) i,
                         QColor(strlist[i]));
    }
    strlist = settings.value(QLatin1String("Palette/disabled")).toStringList();
    if (!strlist.isEmpty()) {
        ++groupCount;
        for (i = 0; i < qMin(strlist.count(), int(QPalette::NColorRoles)); i++)
            pal.setColor(QPalette::Disabled, (QPalette::ColorRole) i,
                         QColor(strlist[i]));
    }
 
/*    if (!appFont) {
        QFont font(QApplication::font());
        QString fontDescription;
        if (fontDescription.isEmpty())
            fontDescription = settings.value(QLatin1String("font")).toString();
        if (!fontDescription .isEmpty()) {
            font.fromString(fontDescription );
            QApplicationPrivate::setSystemFont(font);
        }
    }*/

    // read new QStyle
    QString stylename = settings.value(QLatin1String("style")).toString();
    if (stylename.isEmpty() && QApplicationPrivate::styleOverride.isNull()) {
        QStringList availableStyles = QStyleFactory::keys();
    }

    static QString currentStyleName = stylename;
    if (QCoreApplication::startingUp()) {
        if (!stylename.isEmpty() && QApplicationPrivate::styleOverride.isNull())
            QApplicationPrivate::styleOverride = stylename;
    } else {
        if (currentStyleName != stylename) {
            currentStyleName = stylename;
            QApplication::setStyle(stylename);
        }
    }

    int num =
        settings.value(QLatin1String("doubleClickInterval"),
                       QApplication::doubleClickInterval()).toInt();
    QApplication::setDoubleClickInterval(num);

    num =
        settings.value(QLatin1String("cursorFlashTime"),
                       QApplication::cursorFlashTime()).toInt();
    QApplication::setCursorFlashTime(num);

    num =
        settings.value(QLatin1String("wheelScrollLines"),
                       QApplication::wheelScrollLines()).toInt();
    QApplication::setWheelScrollLines(num);

    QString colorspec = settings.value(QLatin1String("colorSpec"),
                                       QVariant(QLatin1String("default"))).toString();
    if (colorspec == QLatin1String("normal"))
        QApplication::setColorSpec(QApplication::NormalColor);
    else if (colorspec == QLatin1String("custom"))
        QApplication::setColorSpec(QApplication::CustomColor);
    else if (colorspec == QLatin1String("many"))
        QApplication::setColorSpec(QApplication::ManyColor);
    else if (colorspec != QLatin1String("default"))
        colorspec = QLatin1String("default");

/*    QString defaultcodec = settings.value(QLatin1String("defaultCodec"),
                                          QVariant(QLatin1String("none"))).toString();
    if (defaultcodec != QLatin1String("none")) {
        QTextCodec *codec = QTextCodec::codecForName(defaultcodec.toLatin1());
        if (codec)
            QTextCodec::setCodecForTr(codec);
    }*/
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    if (codec) {
     	QTextCodec::setCodecForLocale(codec);
    }           	
    qt_input_mapper = QTextCodec::codecForName("UTF-8");
    

    int w = settings.value(QLatin1String("globalStrut/width")).toInt();
    int h = settings.value(QLatin1String("globalStrut/height")).toInt();
    QSize strut(w, h);
    if (strut.isValid())
        QApplication::setGlobalStrut(strut);

    QStringList effects = settings.value(QLatin1String("GUIEffects")).toStringList();
    QApplication::setEffectEnabled(Qt::UI_General,
                                   effects.contains(QLatin1String("general")));
    QApplication::setEffectEnabled(Qt::UI_AnimateMenu,
                                   effects.contains(QLatin1String("animatemenu")));
    QApplication::setEffectEnabled(Qt::UI_FadeMenu,
                                   effects.contains(QLatin1String("fademenu")));
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo,
                                   effects.contains(QLatin1String("animatecombo")));
    QApplication::setEffectEnabled(Qt::UI_AnimateTooltip,
                                   effects.contains(QLatin1String("animatetooltip")));
    QApplication::setEffectEnabled(Qt::UI_FadeTooltip,
                                   effects.contains(QLatin1String("fadetooltip")));
    QApplication::setEffectEnabled(Qt::UI_AnimateToolBox,
                                   effects.contains(QLatin1String("animatetoolbox")));

  	settings.beginGroup(QLatin1String("Font Substitutions"));
    QStringList fontsubs = settings.childKeys();
    if (!fontsubs.isEmpty()) {
         QStringList::Iterator it = fontsubs.begin();
         for (; it != fontsubs.end(); ++it) {
             QString fam = *it;
             QStringList subs = settings.value(fam).toStringList();
             QFont::insertSubstitutions(fam, subs);
         }
    }
    settings.endGroup();
    
    settings.endGroup(); // Qt

    return true;
}


void QApplicationPrivate::haiku_initialize_style()
{
    if (QApplicationPrivate::app_style)
        return;

    QApplicationPrivate::app_style = QStyleFactory::create(QLatin1String("haiku"));        
}

/*****************************************************************************
  Modal widgets; We have implemented our own modal widget mechanism
  to get total control.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  QApplicationPrivate::enterModal()
        Enters modal state
        Arguments:
            QWidget *widget        A modal widget

  QApplicationPrivate::leaveModal()
        Leaves modal state for a widget
        Arguments:
            QWidget *widget        A modal widget
 *****************************************************************************/

bool QApplicationPrivate::modalState()
{
	//qDebug()<<"Unimplemented: QApplicationPrivate::modalState():"<<app_do_modal;
	return app_do_modal;
}

void QApplicationPrivate::enterModal_sys(QWidget *widget)
{
	qDebug()<<"Unimplemented: QApplicationPrivate::enterModal_sys(). Widget:"<<widget;

    if (!qt_modal_stack)
        qt_modal_stack = new QWidgetList;

//    releaseAutoCapture();
//    ClipCursor(0);
    QWidget *leave = qt_last_mouse_receiver;
    if (!leave)
        leave = QWidget::find(curWin);
    QApplicationPrivate::dispatchEnterLeave(0, leave);
    qt_modal_stack->insert(0, widget);
    app_do_modal = true;
    curWin = 0;
    qt_last_mouse_receiver = 0;
    qt_win_ignoreNextMouseReleaseEvent = false;
}

void QApplicationPrivate::leaveModal_sys(QWidget *widget)
{
	qDebug()<<"Unimplemented: QApplicationPrivate::leaveModal_sys(). Widget:"<<widget;

    if (qt_modal_stack && qt_modal_stack->removeAll(widget)) {
        if (qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
            QPoint p(QCursor::pos());
            app_do_modal = false; // necessary, we may get recursively into qt_try_modal below
            QWidget* w = QApplication::widgetAt(p.x(), p.y());
            QWidget *leave = qt_last_mouse_receiver;
            if (!leave)
                leave = QWidget::find((WId)curWin);
            if (QWidget *grabber = QWidget::mouseGrabber()) {
                w = grabber;
                if (leave == w)
                    leave = 0;
            }
            QApplicationPrivate::dispatchEnterLeave(w, leave); // send synthetic enter event
            curWin = w ? w->effectiveWinId() : 0;
            qt_last_mouse_receiver = w;
        }
        qt_win_ignoreNextMouseReleaseEvent = true;
    }
    app_do_modal = qt_modal_stack != 0;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

#ifdef QT3_SUPPORT
void QApplication::setMainWidget(QWidget *mainWidget)
{
#ifndef QT_NO_DEBUG
    if (mainWidget && mainWidget->parentWidget() && mainWidget->isWindow())
        qWarning("QApplication::setMainWidget: New main widget (%s/%s) "
                  "has a parent",
                  mainWidget->metaObject()->className(), mainWidget->objectName().toLocal8Bit().constData());
#endif
    if (mainWidget)
        mainWidget->d_func()->createWinId();
    QApplicationPrivate::main_widget = mainWidget;
}
#endif

void QApplication::setDoubleClickInterval(int ms)
{
	qDebug("Reimplemented: QApplicationPrivate::setDoubleClickInterval - %d\n", ms);	
#if 0	//change system dblclick interval currently disabled
	QApplicationPrivate::mouse_double_click_time = ms;
	bigtime_t	interval = ms*1000;
	set_click_speed(interval);
#endif
}

int QApplication::doubleClickInterval()
{
	bigtime_t	interval;
	get_click_speed(&interval);
		
	QApplicationPrivate::mouse_double_click_time = (int)(interval/1000);
	
	return QApplicationPrivate::mouse_double_click_time;
}

void QApplication::setKeyboardInputInterval(int ms)
{
    QApplicationPrivate::keyboard_input_time = ms;
}

int QApplication::keyboardInputInterval()
{
    // FIXME: get from the system
    return QApplicationPrivate::keyboard_input_time;
}

void QApplication::setWheelScrollLines(int n)
{
    QApplicationPrivate::wheel_scroll_lines = n;
}

int QApplication::wheelScrollLines()
{
    return QApplicationPrivate::wheel_scroll_lines;
}

void QApplication::setOverrideCursor(const QCursor &cursor)
{
	Q_UNUSED(cursor);
	qDebug("Unimplemented: QApplication::setOverrideCursor\n");
}

void QApplication::restoreOverrideCursor()
{
	qDebug("Unimplemented: QApplication::restoreOverrideCursor\n");
}

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
	Q_UNUSED(effect);
	Q_UNUSED(enable);
//	qDebug("Unimplemented: QApplication::setEffectEnabled\n");
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
	Q_UNUSED(effect);
//	qDebug("Unimplemented: QApplication::isEffectEnabled\n");
	return false;
}

void  QApplication::setCursorFlashTime(int msecs)
{
	QApplicationPrivate::cursor_flash_time = msecs;
}

int QApplication::cursorFlashTime()
{
	return QApplicationPrivate::cursor_flash_time;
}

QWidget *QApplication::topLevelAt(const QPoint &point)
{
    QWidget *found = 0;
    int lowestZ = INT_MAX;
    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; i < list.count(); ++i) {
        QWidget *widget = list.at(i);
        if (widget->isVisible() && !(widget->windowType() == Qt::Desktop)) {
            if (widget->geometry().adjusted(0,0,1,1).contains(point)) {            	    
                    found = widget;			//TODO: check for z-order needed!
                   	if(widget->nativeView()->Window()->IsActive())
                   		break;
            }
        }
    }
    return found;
}

void QApplication::beep()
{
}

void QApplication::alert(QWidget *widget, int duration)
{
}

void QApplicationPrivate::initializeMultitouch_sys()
{
}

void QApplicationPrivate::cleanupMultitouch_sys()
{ }

#ifndef QT_NO_SESSIONMANAGER
QSessionManager::QSessionManager(QApplication * /* app */, QString & /* id */, QString& /* key */)
{

}

QSessionManager::~QSessionManager()
{

}

bool QSessionManager::allowsInteraction()
{
    return false;
}

void QSessionManager::cancel()
{

}
#endif //QT_NO_SESSIONMANAGER
