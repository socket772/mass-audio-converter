#ifndef FFMPEGDELETERS_H
#define FFMPEGDELETERS_H

#include <memory>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace FFmpegUtils {
struct AVFormatInputCloser
{
    void operator()(AVFormatContext *ctx) const
    {
        if (ctx)
            avformat_close_input(&ctx);
    }
};
struct AVFormatOutputFreer
{
    void operator()(AVFormatContext *ctx) const
    {
        if (ctx) {
            if (!(ctx->oformat->flags & AVFMT_NOFILE) && ctx->pb)
                avio_closep(&ctx->pb);
            avformat_free_context(ctx);
        }
    }
};
struct AVCodecContextFreer
{
    void operator()(AVCodecContext *ctx) const
    {
        if (ctx)
            avcodec_free_context(&ctx);
    }
};
struct SwrContextFreer
{
    void operator()(SwrContext *ctx) const
    {
        if (ctx)
            swr_free(&ctx);
    }
};
struct AVFrameFreer
{
    void operator()(AVFrame *frame) const
    {
        if (frame)
            av_frame_free(&frame);
    }
};
struct AVPacketFreer
{
    void operator()(AVPacket *pkt) const
    {
        if (pkt)
            av_packet_free(&pkt);
    }
};

using InputFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatInputCloser>;
using OutputFormatContextPtr = std::unique_ptr<AVFormatContext, AVFormatOutputFreer>;
using CodecContextPtr = std::unique_ptr<AVCodecContext, AVCodecContextFreer>;
using SwrContextPtr = std::unique_ptr<SwrContext, SwrContextFreer>;
using FramePtr = std::unique_ptr<AVFrame, AVFrameFreer>;
using PacketPtr = std::unique_ptr<AVPacket, AVPacketFreer>;
} // namespace FFmpegUtils

#endif // FFMPEGDELETERS_H
