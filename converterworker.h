#ifndef CONVERTERWORKER_H
#define CONVERTERWORKER_H

#include <qthread.h>
#include <qtmetamacros.h>
#include <utils.h>

class ConverterWorker : public QThread
{
protected:
    void run();
signals:
    void progress();
};

#endif // CONVERTERWORKER_H
