#ifndef CONVERTERWORKER_H
#define CONVERTERWORKER_H

#include <qthread.h>
#include <qtmetamacros.h>

class ConverterWorker : public QThread
{
    Q_OBJECT
public:
    explicit ConverterWorker(int threadId, QObject *parent = nullptr)
        : QThread(parent)
        , threadId(threadId) {};
    int threadId;
    std::string filePath;

protected:
    void run();
signals:
    void progress();
};

#endif // CONVERTERWORKER_H
