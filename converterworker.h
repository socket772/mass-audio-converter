#ifndef CONVERTERWORKER_H
#define CONVERTERWORKER_H

#include <qobject.h>
#include <qtmetamacros.h>
#include <utils.h>
class ConverterWorker : public QObject
{
    Q_OBJECT

public:
    ConverterWorker(QAtomicInt *done, QObject *parent = nullptr)
        : QObject(parent)
    {}

signals:
    void progress(int done, int total);
    void finished();
};

#endif // CONVERTERWORKER_H
