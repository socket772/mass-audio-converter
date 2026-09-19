#include "converterworker.h"
#include "utils.h"
#include <qdebug.h>
#include <qlogging.h>
#include <string>

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

        emit progress(threadId, filePath);
    }
    qInfo() << threadId + "-> Finished";
}

int ConverterWorker::convert()
{
    // Cerco il formato del file, inizio dal contenitore
    AVFormatContext *inputContext = nullptr;
    AVFormatContext *outputContext = nullptr;

    // Qui tengo tuttle le stream audio
    std::vector<AudioStream> audioStreams;

    // apro l'header per verificare la leggibilità
    int read_check = avformat_open_input(&inputContext, filePath.c_str(), nullptr, nullptr);
    if (read_check < 0) {
        qWarning().noquote() << threadId + "-> non risco a leggere l'header: " + filePath;
        return 1;
    }

    // verifico i dati della stream questo apre i dati
    read_check = avformat_find_stream_info(inputContext, nullptr);
    if (read_check < 0) {
        qWarning().noquote() << threadId + "-> non risco a leggere la stream: " + filePath;

        // Necessario perchè quando si esce dalla funzione viene
        // distrutto il puntatore in inputContext ma la risorsa viene tenuta aperta
        avformat_close_input(&inputContext);
        return 1;
    }

    // Stampo le informazioni del contenitore
    qInfo().noquote() << QStringLiteral("%1 -> Nome: %2 | Desc: %3 | Num stream: %4")
                             .arg(threadId)
                             .arg(inputContext->iformat->name)
                             .arg(inputContext->iformat->long_name)
                             .arg(inputContext->nb_streams);

    // Carico le stream per il decoding in audiostream

    // Leggo le stream singole
    for (int i = 0; i < inputContext->nb_streams; ++i) {
        AVStream *stream = inputContext->streams[i];
        AVCodecParameters *codecPar = stream->codecpar;
        const AVCodecDescriptor *codecDescriptor = avcodec_descriptor_get(codecPar->codec_id);

        if (codecPar->codec_type == AVMEDIA_TYPE_VIDEO) {
            qWarning().noquote() << QStringLiteral("%1 -> Video non supportato").arg(threadId);
            continue;
        }

        qInfo().noquote() << QStringLiteral("%1 -> Stream: %2 | Tipo: %3 | Codec: %4 | Codec ID: "
                                            "%5 | Sample rate: %6Hz | Canali: %7")
                                 .arg(threadId)
                                 .arg(i)
                                 .arg(av_get_media_type_string(codecPar->codec_type))
                                 .arg(codecDescriptor ? codecDescriptor->name : "sconosciuto")
                                 .arg(codecPar->codec_id)
                                 .arg(codecPar->sample_rate)
                                 .arg(codecPar->ch_layout.nb_channels);

        // Inizializzo il decoder
        const AVCodec *decoder = avcodec_find_decoder(codecPar->codec_id);
        if (!decoder) {
            qWarning().noquote() << QStringLiteral("%1 -> Decoder non trovato").arg(threadId);
            return 1;
        }

        AVCodecContext *codecCtx = avcodec_alloc_context3(decoder);
        if (!codecCtx) {
            qWarning().noquote() << QStringLiteral("%1 -> Decoder non trovato").arg(threadId);
            return 1;
        }

        int ret = avcodec_parameters_to_context(codecCtx, codecPar);
        if (ret < 0) {
            qWarning().noquote() << QStringLiteral(
                                        "%1 -> Errore nella copia dei parametri del codec")
                                        .arg(threadId);
            avcodec_free_context(&codecCtx);
            return 1;
        }

        ret = avcodec_open2(codecCtx, decoder, nullptr);
        if (ret < 0) {
            qWarning().noquote() << QStringLiteral("%1 -> Impossibile aprire il decoder")
                                        .arg(threadId);
            avcodec_free_context(&codecCtx);
            return 1;
        }

        // Uso il decoder
    }

    return 0;
}
