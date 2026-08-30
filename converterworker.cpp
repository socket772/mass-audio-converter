#include "converterworker.h"
#include "utils.h"
#include <qdebug.h>
#include <qlogging.h>
#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

void ConverterWorker::run()
{
    while (true) {
        // Blocco dove accedo al file tramite mutex, cosi viene distrutto subito
        sharedMutex.lock();
        if (filesList.empty()) {
            sharedMutex.unlock();
            break;
        }

        // estrai un file dalla pila (LIFO)
        filePath = filesList.back();
        filesList.pop_back();
        // sblocco il lock
        sharedMutex.unlock();

        // Avvio la conversione
        convert();

        emit progress();
    }
    qInfo() << threadId + "-> Finished";
}

int ConverterWorker::convert()
{
    // Cerco il formato del file, inizio dal contenitore
    AVFormatContext *formatCtx = nullptr;

    // apro l'header per verificare la leggibilità
    int read_check = avformat_open_input(&formatCtx, filePath.c_str(), nullptr, nullptr);
    if (read_check < 0) {
        qWarning().noquote() << threadId + "-> non risco a leggere l'header: " + filePath;
        return -1;
    }

    // verifico i dati della stream
    // questo apre i dati
    read_check = avformat_find_stream_info(formatCtx, nullptr);
    if (read_check < 0) {
        qWarning().noquote() << threadId + "-> non risco a leggere la stream: " + filePath;

        // Necessario perchè quando si esce dalla funzione viene
        // distrutto il puntatore in formatCtx ma la risorsa viene tenuta aperta
        avformat_close_input(&formatCtx);
        return -2;
    }

    // Stampo le informazioni del contenitore
    qInfo().noquote() << QStringLiteral("%1 -> Nome: %2 | Desc: %3 | Num stream: %4")
                             .arg(threadId)
                             .arg(formatCtx->iformat->name)
                             .arg(formatCtx->iformat->long_name)
                             .arg(formatCtx->nb_streams);

    // Leggo le stream singole
    for (int i = 0; i < formatCtx->nb_streams; ++i) {
        AVStream *stream = formatCtx->streams[i];
        AVCodecParameters *codecPar = stream->codecpar;
        const AVCodecDescriptor *codecDescriptor = avcodec_descriptor_get(codecPar->codec_id);

        qInfo().noquote() << threadId + "-> Stream: " + std::to_string(i);
        qInfo().noquote() << QStringLiteral(
                                 "%1 -> Stream: %2 | Tipo: %3 | Codec: %4 | Codec ID: %5")
                                 .arg(threadId)
                                 .arg(i)
                                 .arg(av_get_media_type_string(codecPar->codec_type))
                                 .arg(codecDescriptor ? codecDescriptor->name : "sconosciuto")
                                 .arg(codecPar->codec_id);
    }

    return 0;
}
