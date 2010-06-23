#include "qsystemtrayicon.h"
#include "qsystemtrayicon_p.h"
#include "qdebug.h"
#include "qcolor.h"

#include <OS.h>

#include <Application.h>
#include <Window.h>
#include <Message.h>
#include <Deskbar.h>
#include <View.h>
#include <Roster.h>
#include <Resources.h>
#include <Bitmap.h>
#include <TranslationUtils.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DBAR_SIGNATURE 	"application/x-vnd.Be-TSKB"

status_t SendMessageToReplicant(int32 index, BMessage *msg);

class QSystemTrayIconSys : QWidget
{
public:
    QSystemTrayIconSys(QSystemTrayIcon *object);
    ~QSystemTrayIconSys();
    QSystemTrayIcon *q;
    void createIcon();
        	
	int32	ReplicantId;    
	BBitmap	*icon;
};

QSystemTrayIconSys::QSystemTrayIconSys(QSystemTrayIcon *object)
    : ReplicantId(0), q(object)
{
	
}

QSystemTrayIconSys::~QSystemTrayIconSys()
{
}

void QSystemTrayIconSys::createIcon()
{    
    QIcon qicon = q->icon();
    if (qicon.isNull())
        return;

    QSize size = qicon.actualSize(QSize(16, 16));
    QPixmap pm = qicon.pixmap(size);
    if (pm.isNull())
        return;

    QImage img = pm.toImage();
	
	color_space cspace=B_RGB32;
	switch(img.format()) {
		case QImage::Format_Mono:
			cspace = B_GRAY1;
			break;
		case QImage::Format_Indexed8:
			cspace = B_GRAY8;
			break;
		case QImage::Format_RGB16:
			cspace = B_RGB16;
			break;
		case QImage::Format_RGB32:
		default:
			cspace = B_RGB32;
			break;
	}
	
	qDebug() << "QIcon() " << img.width() << img.height();
	
	icon = pm.toHaikuBitmap();			

	BMessage	bits(B_ARCHIVED_OBJECT);
	icon->Archive(&bits);	
	BMessage mes('BITS');
	mes.AddMessage("icon",&bits);
	bits.MakeEmpty();
		
	SendMessageToReplicant(ReplicantId,&mes);    	

	mes.PrintToStream();	
}

BMessenger GetMessenger(void)
{
	BMessenger aResult;
	status_t aErr = B_OK;
	BMessenger aDeskbar(DBAR_SIGNATURE, -1, &aErr);
	if (aErr != B_OK)return aResult;

	BMessage aMessage(B_GET_PROPERTY);
	
	aMessage.AddSpecifier("Messenger");
	aMessage.AddSpecifier("Shelf");
	aMessage.AddSpecifier("View", "Status");
	aMessage.AddSpecifier("Window", "Deskbar");
	
	BMessage aReply;

	if (aDeskbar.SendMessage(&aMessage, &aReply, 1000000, 1000000) == B_OK)
		aReply.FindMessenger("result", &aResult);
	return aResult;
}


status_t SendMessageToReplicant(int32 index, BMessage *msg)
{
	BMessage aReply;
	status_t aErr = B_OK;
	
	msg->AddInt32( "what2", msg->what );
	msg->what = B_SET_PROPERTY;

	BMessage	uid_specifier(B_ID_SPECIFIER);
	
	msg->AddSpecifier("View");
	uid_specifier.AddInt32("id", index);
	uid_specifier.AddString("property", "Replicant");
	msg->AddSpecifier(&uid_specifier);
		
	aErr = GetMessenger().SendMessage( msg, (BHandler*)NULL, 1000000 );
	return aErr;
}

int32	qsystray_exec_manager(char *command)
{
   FILE *fpipe;
   char line[256];
   if ( !(fpipe = (FILE*)popen(command,"r")) )
   		return -1;

   fgets( line, sizeof line, fpipe);
   pclose(fpipe);
   
   int res = atoi(line);
   return res;
}

int32 DeskBarLoadIcon(team_id tid)
{
	char cmd[256];
	sprintf(cmd,"qsystray %d",tid);	
	int32 id = qsystray_exec_manager(cmd);
	return id;
}

int32 DeskBarLoadIcon(void)
{
	thread_info threadInfo;
	status_t error = get_thread_info(find_thread(NULL), &threadInfo);
	if (error != B_OK) {
		fprintf(stderr, "Failed to get info for the current thread: %s\n",
			strerror(error));
			return -1;	
	}
	team_id sTeam = threadInfo.team;
	
	return DeskBarLoadIcon(sTeam);
}

void DeskbarRemoveIcon(int32 id)
{
	BDeskbar deskbar;
	deskbar.RemoveItem(id);
}


void QSystemTrayIconPrivate::install_sys()
{
	fprintf(stderr, "Reimplemented: QSystemTrayIconPrivate::install_sys \n");
    Q_Q(QSystemTrayIcon);
    if (!sys) {
        sys = new QSystemTrayIconSys(q);
        sys->ReplicantId = DeskBarLoadIcon();
        sys->createIcon();
    }
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
	fprintf(stderr, "Reimplemented: QSystemTrayIconPrivate::remove_sys \n");
	if(!sys)
		return;
	if(sys->ReplicantId>0)
		DeskbarRemoveIcon(sys->ReplicantId);
    
    delete sys;
    sys = 0;		
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
	fprintf(stderr, "Reimplemented:  QSystemTrayIconPrivate::updateIcon_sys\n");

    if (!sys)
        return;
        
    sys->createIcon();
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
	fprintf(stderr, "Reimplemented:  QSystemTrayIconPrivate::isSystemTrayAvailable_sys\n");
	return true;
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
    return false;
}
