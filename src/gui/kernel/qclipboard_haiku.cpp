#include "qplatformdefs.h"
#include "qclipboard.h"
#include "qdebug.h"
#include "qtextcodec.h"
#include "qclipboard_p.h"
#include "qstringlist.h"
#include <stdio.h>

#include <String.h>
#include <Clipboard.h>

bool QClipboard::event(QEvent *e)
{
	return true;
}

void QClipboard::connectNotify(const char *)
{ }

void QClipboard::ownerDestroyed()
{ }

const QMimeData *QClipboard::mimeData(Mode mode) const
{
	extern QTextCodec *qt_input_mapper; // from qapplication_haiku.cpp
	
	QMimeData *md = new QMimeData();
	if(mode == Clipboard) {		
		BMessage* clip = (BMessage *)NULL;
	  	if (be_clipboard->Lock()) {
	    	if( clip = be_clipboard->Data()) {
	    		BMessage *msg = (BMessage*)(be_clipboard->Data());
	    		
				char *name;
				uint32 type;
				int32 count;

				for ( int i = 0; msg->GetInfo(B_MIME_TYPE, i, &name, &type, &count) == B_OK; i++ ) {
					const void *data;
					ssize_t dataLen = 0;
					qDebug() << "mimeData " << name;
					status_t stat = msg->FindData(name,B_MIME_TYPE,&data,&dataLen);
					if(dataLen && stat==B_OK)	{
						QString mime(name);
						if(mime=="text/plain") {
							QString text((const char*)data);
						    if (qt_input_mapper)
        						text = qt_input_mapper->toUnicode((const char*)data, dataLen, 0);
							md->setText(text);
						} else if(mime=="text/html") {
							QString html((const char*)data);
						    if (qt_input_mapper)
        						html = qt_input_mapper->toUnicode((const char*)data, dataLen, 0);
							md->setHtml(html);
						} else {
							QByteArray clip_data((const char*)data, dataLen);
							md->setData(mime,clip_data);
						}

					}
				}								
				be_clipboard->Unlock();
	    	}
		}			
	} 
	return md;
}

void QClipboard::setMimeData(QMimeData *src, Mode mode)
{	
	if(mode != Clipboard)
		return;
	if (be_clipboard->Lock()) {
		be_clipboard->Clear();
		if (src){
			BMessage* clip = (BMessage *)NULL;
	    	if( clip = be_clipboard->Data()) {
	    		QStringList formats = src->formats();
				for(int f = 0; f < formats.size(); ++f) {
            		QString mimeType = formats.at(f);	    			
            		qDebug() << "setMimeData " << mimeType;         		
					clip->AddData(mimeType.toUtf8(), B_MIME_TYPE, src->data(mimeType).data(), src->data(mimeType).count());
	    		}
	    	}
		}
		be_clipboard->Commit();
	   	be_clipboard->Unlock();
	}
}

void QClipboard::clear(Mode mode)
{
    setMimeData(0, mode);
}

bool QClipboard::supportsMode(QClipboard::Mode mode) const
{
	return (mode == Clipboard);
}

bool QClipboard::ownsMode(QClipboard::Mode) const
{
    fprintf(stderr, "Unimplemented: QClipboard::ownsMode\n");
    return false;
}
