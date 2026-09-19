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
    int convert();
    // Secondo duckai
    //     inputFormat
    //      └── inputStream
    //          └── decoder
    //              └── decodedFrame
    //                  └── resampler
    //                      └── convertedFrame
    //                          └── encoder
    //                              └── outputStream
    struct AudioStream
    {
        int inputIndex = -1;
        int outputIndex = -1;
        // Salva la stream singola
        AVStream *inputStream = nullptr;
        AVStream *outputStream = nullptr;
        // Serve un decoder/encoder per stream
        AVCodecContext *decoder = nullptr;
        AVCodecContext *encoder = nullptr;
        // Conversione del formato audio
        SwrContext *resampler = nullptr;
        // Audio convertito
        AVFrame *decodedFrame = nullptr;
        AVFrame *convertedFrame = nullptr;
    };

signals:
    void progress(std::string threadId, std::string filePath);
};

#endif // CONVERTERWORKER_H
