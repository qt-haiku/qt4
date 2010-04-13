#include "qapplication.h"
#include "qsound.h"
#include "qsound_p.h"

class QAuServerNull : public QAuServer
{
public:
    QAuServerNull(QObject* parent);

    void play(const QString&) { }
    void play(QSound*s) { while(decLoop(s) > 0) /* nothing */ ; }
    void stop(QSound*) { }
    bool okay() { return false; }
};

QAuServerNull::QAuServerNull(QObject* parent)
    : QAuServer(parent)
{
}

QAuServer* qt_new_audio_server()
{
//	fprintf(stderr, "Unimplemented: qt_new_audio_server()\n");
	return new QAuServerNull(qApp); 
}

