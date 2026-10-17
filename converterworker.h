#ifndef CONVERTERWORKER_H
#define CONVERTERWORKER_H

#include <qthread.h>
#include <qtmetamacros.h>
#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

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
    // Funzioni
    int convert();

signals:
    void progress(std::string threadId, std::string filePath);
};

#endif // CONVERTERWORKER_H
