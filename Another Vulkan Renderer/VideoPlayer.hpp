#pragma once
#ifdef __cplusplus
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#endif
#include <string>
#include <span>
#include "context.h"

class VideoPlayer{
public:
    VideoPlayer(const std::string& name, avr::Context& other);
    ~VideoPlayer();
    std::span<uint8_t> getFrame(std::uint32_t& width, std::uint32_t& height);
    AVCodecContext* pCodecContext{ nullptr };
    AVFrame* pFrame{ nullptr };

    class Encoder{
    public:
        Encoder(avr::Context& other);
        ~Encoder();
    
        avr::Context& ctx;
        AVFormatContext* format_context = nullptr;
        AVStream* stream = nullptr;
        AVCodecContext* codec_context = nullptr;
        AVFrame* frame = nullptr;
        SwsContext* sws_context = nullptr;
        const AVCodec* codec;
        AVPacket* packet;
        // for now just use this for present timing
        uint32_t frame_index = 0;

        void open(const std::string& fileName, int width, int height);
        void writeFrame(std::vector<uint8_t>& data);
    };
    Encoder encoder;
private:
    AVFormatContext* pFormatContext{ nullptr };
    AVCodecParameters* codecParam{ nullptr };
    const AVCodec* pCodec{ nullptr };
    AVPacket* pPacket{ nullptr };
    AVFrame* rgbVideoFrame{ nullptr };
    //AVFrame* pFrame{ nullptr };
    const std::string fileName;

};

