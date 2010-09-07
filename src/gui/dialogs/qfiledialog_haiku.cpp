/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
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

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include <private/qfiledialog_p.h>
#include <qapplication.h>
#include <private/qapplication_p.h>
#include <qdesktopservices.h>
#include <qglobal.h>
#include <qregexp.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qlibrary.h>
#include "qeventloop.h"

#ifndef QT_NO_THREAD
#  include <private/qmutexpool_p.h>
#endif

#include <FilePanel.h>
#include <Directory.h>
#include <Entry.h>
#include <Node.h>
#include <Path.h>
#include <TextControl.h>

QT_BEGIN_NAMESPACE

extern const char* qt_file_dialog_filter_reg_exp; // defined in qfiledialog.cpp
extern QStringList qt_make_filter_list(const QString &filter);

const int maxNameLen = 1023;
const int maxMultiLen = 65535;

class PanelLooper : public BLooper {
public:
							PanelLooper();
	virtual					~PanelLooper();
	virtual	void			MessageReceived(BMessage* message);

	int						Wait(BFilePanel *panel);
	
	QString					GetFilename();
	QStringList				GetFilenames();
		
private:
	int 					state;
	QString					filename;
	QStringList				filenames;
	entry_ref				fRef;	
};


PanelLooper::PanelLooper() : BLooper("PanelLooper"), state(B_CANCEL) { }
PanelLooper::~PanelLooper() { }

int
PanelLooper::Wait(BFilePanel *panel)
{
    QEventLoop::ProcessEventsFlags flags;
    flags |= QEventLoop::WaitForMoreEvents;
    while(panel->IsShowing()) {
    	QCoreApplication::processEvents(flags);
    	snooze(250);
    }	
    return state;
}

QString
PanelLooper::GetFilename()
{
	return filename;
}

QStringList
PanelLooper::GetFilenames()
{
	return filenames;
}

void
PanelLooper::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_SAVE_REQUESTED:
			{
				entry_ref ref;
				const char *name;
				message->FindRef("directory", &ref);
				BDirectory  dir(&ref);
				BPath path(&dir, NULL, false);
				message->FindString("name", &name);
				path.Append(name);
				filename = QString::fromUtf8(path.Path());
				filenames.append(filename);
								
				state = B_OK;
			}
			break;
		case B_REFS_RECEIVED:
			{
				uint32 type;
       			int32 count;				
       			
				message->GetInfo("refs", &type, &count);
       			if ( type != B_REF_TYPE || count <= 0)
           			return;				

				for ( long i = --count; i >= 0; i-- ) {
           			if ( message->FindRef("refs", i, &fRef) == B_OK ) {
						BPath path(&fRef);
				    	filename = QString::fromUtf8(path.Path());
				    	filenames.append(filename);   				
           			}
				}
								    	
		    	state = B_OK;
			}
			break;
		case B_CANCEL:
			{
				if(state!=B_OK) {
					state = B_CANCEL;
					filename.clear();
					filenames.clear();
				}
			}
			break;
		default:
			break;
	}
}


// Returns the wildcard part of a filter.
static QString qt_haiku_extract_filter(const QString &rawFilter)
{
    QString result = rawFilter;
    QRegExp r(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    int index = r.indexIn(result);
    if (index >= 0)
        result = r.cap(2);
    QStringList list = result.split(QLatin1Char(' '));
    for(QStringList::iterator it = list.begin(); it < list.end(); ++it) {
        if (*it == QLatin1String("*")) {
            *it = QLatin1String("*.*");
            break;
        }
    }
    return list.join(QLatin1String(";"));
}

static QStringList qt_haiku_make_filters_list(const QString &filter)
{
    QString f(filter);

    if (f.isEmpty())
        f = QFileDialog::tr("All Files (*.*)");

    return qt_make_filter_list(f);
}

// Makes a NUL-oriented Haiku filter from a Qt filter.
static QString qt_haiku_filter(const QString &filter)
{
    QStringList filterLst = qt_haiku_make_filters_list(filter);
    QStringList::Iterator it = filterLst.begin();
    QString haikufilters;
    for (; it != filterLst.end(); ++it) {
        QString subfilter = *it;
        if (!subfilter.isEmpty()) {
            haikufilters += subfilter;
            haikufilters += QChar();
            haikufilters += qt_haiku_extract_filter(subfilter);
            haikufilters += QChar();
        }
    }
    haikufilters += QChar();
    return haikufilters;
}

static QString qt_haiku_selected_filter(const QString &filter, int idx)
{
    return qt_haiku_make_filters_list(filter).at((int)idx - 1);
}


QString qt_haiku_get_open_file_name(const QFileDialogArgs &args,
                                  QString *initialDirectory,
                                  QString *selectedFilter)
{	
    QString result;

	QString title = args.caption;
    QString isel = args.selection;

    if (initialDirectory && initialDirectory->left(5) == QLatin1String("file:"))
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.absolutePath();
        if (isel.isEmpty())
            isel = fi.fileName();
    }

    if (!fi.exists() || *initialDirectory==QDir::homePath())
        *initialDirectory = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);

/*    int selFilIdx = 0;

    int idx = 0;
    if (selectedFilter) {
        QStringList filterLst = qt_haiku_make_filters_list(args.filter);
        idx = filterLst.indexOf(*selectedFilter);
    }*/

	PanelLooper *looper = new PanelLooper();
	looper->Run();	

    BFilePanel *openpanel = new BFilePanel(B_OPEN_PANEL,NULL,NULL,0,false,NULL,NULL,true,true);
    openpanel->SetTarget(BMessenger(looper));    
	if(!title.isEmpty()) {
		openpanel->Window()->SetTitle((title.toUtf8()).data());
	}      
    QByteArray dirpath = initialDirectory->toUtf8();
    openpanel->SetPanelDirectory(dirpath.data());    
    openpanel->Show();
    
	looper->Wait(openpanel);
	result = looper->GetFilename();
	
	delete openpanel;

	looper->Lock();
	looper->Quit();	

    if (result.isEmpty())
        return result;

    fi = result;
    *initialDirectory = fi.path();
	    
	return  fi.absoluteFilePath();;
}

QString qt_haiku_get_save_file_name(const QFileDialogArgs &args,
                                  QString *initialDirectory,
				  QString *selectedFilter)
{
    QString result;

	QString title = args.caption;
    QString isel = args.selection;
    
    if (initialDirectory && initialDirectory->left(5) == QLatin1String("file:"))
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.absolutePath();
        if (isel.isEmpty())
            isel = fi.fileName();
    }
	
    if (!fi.exists() || *initialDirectory==QDir::homePath())
        *initialDirectory = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
/*
    int selFilIdx = 0;

    int idx = 0;
    if (selectedFilter) {
        QStringList filterLst = qt_haiku_make_filters_list(args.filter);
        idx = filterLst.indexOf(*selectedFilter);
    }

    QString defaultSaveExt;
    if (selectedFilter && !selectedFilter->isEmpty()) {
        defaultSaveExt = qt_haiku_extract_filter(*selectedFilter);
        // make sure we only have the extension
        int firstDot = defaultSaveExt.indexOf(QLatin1Char('.'));
        if (firstDot != -1) {
            defaultSaveExt.remove(0, firstDot + 1);
        } else {
            defaultSaveExt.clear();
        }
    }*/
    
	PanelLooper *looper = new PanelLooper();
	looper->Run();	    

    BFilePanel *savepanel = new BFilePanel(B_SAVE_PANEL,NULL,NULL,0,false,NULL,NULL,true,true);
    savepanel->SetTarget(BMessenger(looper));    
	if(!title.isEmpty()) {
		savepanel->Window()->SetTitle((title.toUtf8()).data());
	}     
    QByteArray dirpath = initialDirectory->toUtf8();
    savepanel->SetPanelDirectory(dirpath.data());    
	savepanel->SetSaveText((isel.toUtf8()).data());
    savepanel->Show();
    
	looper->Wait(savepanel);
	result = looper->GetFilename();	
	
	delete savepanel;
	
	looper->Lock();
	looper->Quit();	
	
    if (result.isEmpty())
        return result;

    fi = result;
    *initialDirectory = fi.path();
/*    if (selectedFilter)
        *selectedFilter = qt_haiku_selected_filter(args.filter, selFilIdx);*/
    return fi.absoluteFilePath();
}

QStringList qt_haiku_get_open_file_names(const QFileDialogArgs &args,
                                       QString *initialDirectory,
                                       QString *selectedFilter)
{
	QStringList result;
	
	QString title = args.caption;
    QString isel = args.selection;

    if (initialDirectory && initialDirectory->left(5) == QLatin1String("file:"))
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.absolutePath();
        if (isel.isEmpty())
            isel = fi.fileName();
    }

    if (!fi.exists() || *initialDirectory==QDir::homePath())
        *initialDirectory = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);

    int selFilIdx = 0;

/*    int idx = 0;
    if (selectedFilter) {
        QStringList filterLst = qt_haiku_make_filters_list(args.filter);
        idx = filterLst.indexOf(*selectedFilter);
    }*/
    
	PanelLooper *looper = new PanelLooper();
	looper->Run();    

    BFilePanel *openpanel = new BFilePanel(B_OPEN_PANEL,NULL,NULL,0,true,NULL,NULL,true,true);
    openpanel->SetTarget(BMessenger(looper));
	if(!title.isEmpty()) {
		openpanel->Window()->SetTitle((title.toUtf8()).data());
	}    
    QByteArray dirpath = initialDirectory->toUtf8();
    openpanel->SetPanelDirectory(dirpath.data());
    openpanel->Show();
    
	looper->Wait(openpanel);
	result = looper->GetFilenames();

	delete openpanel;

	looper->Lock();
	looper->Quit();		    

    if (!result.isEmpty()) {
        *initialDirectory = fi.path();    // only save the path if there is a result
    }
    return result;
}

class DirectoryFilter : public BRefFilter {
public:
	DirectoryFilter() {};
	virtual bool Filter(const entry_ref* ref,
		BNode* node, struct stat_beos* st, const char* filetype)
	{
		return node->IsDirectory();
	}
};


QString qt_haiku_get_existing_directory(const QFileDialogArgs &args)
{
    QString initDir = QDir::toNativeSeparators(args.directory);
    QString result;
    
    QString title = args.caption;
    
	PanelLooper *looper = new PanelLooper();
	looper->Run();	
	    
	BFilePanel *dirpanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL,	B_DIRECTORY_NODE, true, NULL, NULL, true, true);	
 	dirpanel->SetTarget(BMessenger(looper));  	
	dirpanel->SetButtonLabel(B_DEFAULT_BUTTON, "Select");
	if(!title.isEmpty()) {
		dirpanel->Window()->SetTitle((title.toUtf8()).data());
	}

	BRefFilter *filter;
	filter = new DirectoryFilter;
	dirpanel->SetRefFilter(filter);   
	dirpanel->Show();
	
	looper->Wait(dirpanel);
	result = looper->GetFilename();
	    
    delete dirpanel;
    
	looper->Lock();
	looper->Quit();  
    
    return result;
}


QT_END_NAMESPACE

#endif
