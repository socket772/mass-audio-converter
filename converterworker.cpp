#include "converterworker.h"
#include "ffmpegdeleters.h"
#include "utils.h"
#include <qdebug.h>
#include <qlogging.h>
#include <string>

using namespace FFmpegUtils;

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
    // Creazione contesto di input
    AVFormatContext *rawInputCtx;
    if (avformat_open_input(&rawInputCtx, filePath.c_str(), nullptr, nullptr) < 0) {
        qWarning().noquote() << QString::fromStdString(threadId)
                                    + " -> Impossibile aprire l'header: "
                                    + QString::fromStdString(filePath);
        return 1;
    }
    // Da questo momento la risorsa è gestita dal RAII
    InputFormatContextPtr inputContext(rawInputCtx);

    if (avformat_find_stream_info(inputContext.get(), nullptr) < 0) {
        qWarning().noquote() << QString::fromStdString(threadId)
                                    + " -> Impossibile leggere gli stream: "
                                    + QString::fromStdString(filePath);
        return 1; // RAII chiuderà automaticamente inputContext!
    }

    qInfo().noquote() << QStringLiteral("%1 -> Nome: %2 | Desc: %3 | Num stream: %4")
                             .arg(QString::fromStdString(threadId))
                             .arg(inputContext->iformat->name)
                             .arg(inputContext->iformat->long_name)
                             .arg(inputContext->nb_streams);

    // Creazione contesto di output
    AVFormatContext *rawOutputCtx;
    if (avformat_alloc_output_context2(&rawOutputCtx, nullptr, nullptr, filePath.c_str()) < 0
        || !rawOutputCtx) {
        qWarning().noquote() << QString::fromStdString(threadId)
                                    + " -> Impossibile allocare l'output context";
        return 1; // RAII pulisce automaticamente inputContext!
    }
    OutputFormatContextPtr outputContext(rawOutputCtx);

    // setup dei vettori per le stream singole da convertire
    std::unordered_map<int, size_t> streamMap;
    std::vector<int> outStreamIndices;
    std::vector<CodecContextPtr> decoders;
    std::vector<CodecContextPtr> encoders;
    std::vector<SwrContextPtr> resamplers;

    // Conversione delle stream audio
    for (int i = 0; i < static_cast<int>(inputContext->nb_streams); ++i) {
        AVStream *inStream = inputContext->streams[i];
        AVCodecParameters *codecPar = inStream->codecpar;

        // Filtro le stream per solo quelle audio
        if (codecPar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }

        // DECODER
        // cerca il decoder di default
        const AVCodec *decoder = avcodec_find_decoder(codecPar->codec_id);
        if (!decoder)
            continue;

        // creo il contesto del decoder. però è vuoto
        CodecContextPtr decCtx(avcodec_alloc_context3(decoder));
        if (!decCtx)
            continue;

        // Configura le impostazioni del decoder, copiando quelle della stream
        // Configuro anche la scala di tempo
        avcodec_parameters_to_context(decCtx.get(), codecPar);
        decCtx->time_base = inStream->time_base;

        if (avcodec_open2(decCtx.get(), decoder, nullptr) < 0)
            continue;

        // ENCODER
        // cerca l'encoder di default
        const AVCodec *encoder = avcodec_find_encoder(outputContext->oformat->audio_codec);
        if (!encoder)
            continue;

        // Creazione stream vuota di output
        AVStream *outStream = avformat_new_stream(outputContext.get(), nullptr);
        if (!outStream)
            continue;

        // configurazione dell'encoder
        CodecContextPtr encCtx(avcodec_alloc_context3(encoder));
        if (!encCtx)
            continue;

        // copio i dati nell'encoder per averli uguali al decoder
        encCtx->sample_rate = decCtx->sample_rate;
        av_channel_layout_copy(&encCtx->ch_layout, &decCtx->ch_layout);

        // FFmpeg 7.0+ API per la scelta del sample_fmt
        //
        const enum AVSampleFormat *supported_sample_fmts = nullptr;
        int num_sample_fmts = 0;
        avcodec_get_supported_config(nullptr,
                                     encoder,
                                     AV_CODEC_CONFIG_SAMPLE_FORMAT,
                                     0,
                                     (const void **) &supported_sample_fmts,
                                     &num_sample_fmts);

        encCtx->sample_fmt = (supported_sample_fmts && num_sample_fmts > 0)
                                 ? supported_sample_fmts[0]
                                 : AV_SAMPLE_FMT_FLTP;

        encCtx->time_base = (AVRational) {1, encCtx->sample_rate};
        outStream->time_base = encCtx->time_base;

        if (outputContext->oformat->flags & AVFMT_GLOBALHEADER) {
            encCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        if (avcodec_open2(encCtx.get(), encoder, nullptr) < 0)
            continue;

        avcodec_parameters_from_context(outStream->codecpar, encCtx.get());

        // --- RESAMPLER ---
        SwrContext *rawSwr = nullptr;
        swr_alloc_set_opts2(&rawSwr,
                            &encCtx->ch_layout,
                            encCtx->sample_fmt,
                            encCtx->sample_rate,
                            &decCtx->ch_layout,
                            decCtx->sample_fmt,
                            decCtx->sample_rate,
                            0,
                            nullptr);

        SwrContextPtr swr(rawSwr);
        if (swr_init(swr.get()) < 0)
            continue;

        // Salva la traccia configurata nei vettori RAII
        size_t trackIdx = decoders.size();
        streamMap[i] = trackIdx;
        outStreamIndices.push_back(outStream->index);
        decoders.push_back(std::move(decCtx));
        encoders.push_back(std::move(encCtx));
        resamplers.push_back(std::move(swr));
    }

    if (decoders.empty()) {
        qWarning().noquote() << QString::fromStdString(threadId)
                                    + " -> Nessuna traccia audio convertibile trovata";
        return 1; // Rilascia in automatico tutte le risorse allocate fino a qui
    }

    // 4. APERTURA FILE E SCRITTURA HEADER
    if (!(outputContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&outputContext->pb, filePath.c_str(), AVIO_FLAG_WRITE) < 0) {
            qWarning().noquote() << QString::fromStdString(threadId)
                                        + " -> Impossibile aprire il file di output";
            return 1;
        }
    }

    if (avformat_write_header(outputContext.get(), nullptr) < 0) {
        qWarning().noquote() << QString::fromStdString(threadId)
                                    + " -> Impossibile scrivere l'header di output";
        return 1;
    }

    // 5. BUFFER E FRAME RAII
    PacketPtr inPacket(av_packet_alloc());
    PacketPtr outPacket(av_packet_alloc());
    FramePtr decodedFrame(av_frame_alloc());
    FramePtr convertedFrame(av_frame_alloc());

    // LOOP DI CONVERSIONE
    while (av_read_frame(inputContext.get(), inPacket.get()) >= 0) {
        auto it = streamMap.find(inPacket->stream_index);

        if (it != streamMap.end()) {
            size_t idx = it->second;
            AVCodecContext *decCtx = decoders[idx].get();
            AVCodecContext *encCtx = encoders[idx].get();
            SwrContext *swr = resamplers[idx].get();
            int outStreamIdx = outStreamIndices[idx];
            AVStream *inStream = inputContext->streams[inPacket->stream_index];

            if (avcodec_send_packet(decCtx, inPacket.get()) >= 0) {
                while (avcodec_receive_frame(decCtx, decodedFrame.get()) >= 0) {
                    convertedFrame->nb_samples = decodedFrame->nb_samples;
                    convertedFrame->format = encCtx->sample_fmt;
                    av_channel_layout_copy(&convertedFrame->ch_layout, &encCtx->ch_layout);
                    convertedFrame->sample_rate = encCtx->sample_rate;
                    av_frame_get_buffer(convertedFrame.get(), 0);

                    swr_convert(swr,
                                convertedFrame->data,
                                convertedFrame->nb_samples,
                                (const uint8_t **) decodedFrame->data,
                                decodedFrame->nb_samples);

                    convertedFrame->pts = av_rescale_q(decodedFrame->pts,
                                                       inStream->time_base,
                                                       encCtx->time_base);

                    if (avcodec_send_frame(encCtx, convertedFrame.get()) >= 0) {
                        while (avcodec_receive_packet(encCtx, outPacket.get()) >= 0) {
                            outPacket->stream_index = outStreamIdx;
                            av_packet_rescale_ts(outPacket.get(),
                                                 encCtx->time_base,
                                                 outputContext->streams[outStreamIdx]->time_base);
                            av_interleaved_write_frame(outputContext.get(), outPacket.get());
                            av_packet_unref(outPacket.get());
                        }
                    }
                    av_frame_unref(convertedFrame.get());
                    av_frame_unref(decodedFrame.get());
                }
            }
        }
        av_packet_unref(inPacket.get());
    }

    // 6. FLUSH DEGLI ENCODER
    for (size_t idx = 0; idx < encoders.size(); ++idx) {
        avcodec_send_frame(encoders[idx].get(), nullptr);
        while (avcodec_receive_packet(encoders[idx].get(), outPacket.get()) >= 0) {
            outPacket->stream_index = outStreamIndices[idx];
            av_packet_rescale_ts(outPacket.get(),
                                 encoders[idx]->time_base,
                                 outputContext->streams[outStreamIndices[idx]]->time_base);
            av_interleaved_write_frame(outputContext.get(), outPacket.get());
            av_packet_unref(outPacket.get());
        }
    }

    av_write_trailer(outputContext.get());

    qInfo().noquote() << QString::fromStdString(threadId)
                             + " -> Conversione completata con successo!";

    // Tutti gli smart pointer e vettori RAII verranno distrutti
    // automaticamente al return, liberando la memoria C di FFmpeg.
    return 0;
}
