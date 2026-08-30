#ifndef CONVERTERWORKER_H
#define CONVERTERWORKER_H

#include <qthread.h>
#include <qtmetamacros.h>

class ConverterWorker : public QThread
{
    Q_OBJECT
public:
    explicit ConverterWorker(std::string threadId, QObject *parent = nullptr)
        : QThread(parent)
        , threadId(threadId) {};
    std::string threadId;
    std::string filePath;

protected:
    void run();

private:
    int convert();

signals:
    void progress();
};

#endif // CONVERTERWORKER_H
