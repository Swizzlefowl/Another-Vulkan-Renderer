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
class VideoPlayer{
public:
    VideoPlayer(const std::string& name);
    ~VideoPlayer();
    std::span<uint8_t> getFrame(std::uint32_t& width, std::uint32_t& height);
    AVCodecContext* pCodecContext{ nullptr };
    AVFrame* pFrame{ nullptr };
private:
    AVFormatContext* pFormatContext{ nullptr };
    AVCodecParameters* codecParam{ nullptr };
    const AVCodec* pCodec{ nullptr };
    AVPacket* pPacket{ nullptr };
    AVFrame* rgbVideoFrame{ nullptr };
    //AVFrame* pFrame{ nullptr };
    const std::string fileName;

};

