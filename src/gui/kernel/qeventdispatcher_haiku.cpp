#include <private/qabstracteventdispatcher_p.h>
#include "qeventdispatcher_haiku_p.h"
#include <private/qcoreapplication_p.h>
#include <private/qthread_p.h>
#include <private/qmutexpool_p.h>

#include <Application.h>
#include <stdio.h>

// Debugging part

#define DEBUG_HAIKU_LEVEL 0
// 1 Unimplemented
// 2-4 nothing
// 5 reimplemented

#if defined(DEBUG_HAIKU_LEVEL) && DEBUG_HAIKU_LEVEL > 0
void debugprint(int level, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	if (DEBUG_HAIKU_LEVEL >= level)
		vfprintf(stderr, msg, args);
	va_end(args);
}
#else
#define debugprint(...)
#endif

class QEventDispatcherHaikuPrivate : public QEventDispatcherUNIXPrivate
{
	QEventDispatcherHaikuPrivate() : QEventDispatcherUNIXPrivate() { }
	~QEventDispatcherHaikuPrivate() { }
};

QEventDispatcherHaiku::QEventDispatcherHaiku(QObject *parent)
		: QEventDispatcherUNIX(*new QEventDispatcherUNIXPrivate, parent)
{
	debugprint(5, "Reimp: QEventDispatcherHaiku::QEventDispatcherHaiku \n");
}
QEventDispatcherHaiku::~QEventDispatcherHaiku()
{
	debugprint(5, "Reimp: QEventDispatcherHaiku::~QEventDispatcherHaiku \n");
}

bool  
QEventDispatcherHaiku::processEvents(QEventLoop::ProcessEventsFlags  
flags)
{
	Q_D(QEventDispatcherHaiku);
	debugprint(5, "Reimp: QEventDispatcherHaiku::processEvents\n");

    d->interrupt = false;
    emit awake();

    bool canWait = false;
    bool retVal = false;
    do {
        QCoreApplicationPrivate::sendPostedEvents(0, 0, d->threadData);
        QThreadData *data = d->threadData;
        canWait = (//!retVal
                    data->canWait
                   && !d->interrupt
                   && (flags & QEventLoop::WaitForMoreEvents));
	} while (false);//canWait);

	if (!d->interrupt) {
//		fprintf(stderr, "Entering Unix processEvents\n");
		bool result = QEventDispatcherUNIX::processEvents(flags);
//		fprintf(stderr, "Exiting Unix processEvents\n");
		return result;
	}
	return true;
}
bool QEventDispatcherHaiku::hasPendingEvents()
{
	debugprint(5, "Reimp: QEventDispatcherHaiku::hasPendingEvents\n");
	return false;
}

void QEventDispatcherHaiku::flush()
{
	debugprint(5, "Reimp: QEventDispatcherHaiku::flush\n");
}

void QEventDispatcherHaiku::startingUp()
{
	debugprint(5, "Reimp: QEventDispatcherHaiku::startingUp\n");
}
void QEventDispatcherHaiku::closingDown()
{
	debugprint(5, "Reimp: QEventDispatcherHaiku::closingDown\n");
}

