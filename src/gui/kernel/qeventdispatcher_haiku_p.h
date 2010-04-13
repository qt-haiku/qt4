#ifndef FEASOIFESWR
#define FEASOIFESWR
#include <private/qabstracteventdispatcher_p.h>
#include <private/qeventdispatcher_unix_p.h>

class QEventDispatcherHaikuPrivate;

class QEventDispatcherHaiku : public QEventDispatcherUNIX
{
	//Q_OBJECT
	Q_DECLARE_PRIVATE(QEventDispatcherHaiku)

public:
	explicit QEventDispatcherHaiku(QObject *parent = 0);
	~QEventDispatcherHaiku();

	bool processEvents(QEventLoop::ProcessEventsFlags flags);
	bool hasPendingEvents();

	void flush();

	void startingUp();
	void closingDown();

protected:
};

#endif
