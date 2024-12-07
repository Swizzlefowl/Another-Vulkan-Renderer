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
#include <vector>
class VideoPlayer{
public:
    VideoPlayer(const std::string& name);
    ~VideoPlayer();
    std::vector<uint8_t> getFrame(std::uint32_t& width, std::uint32_t& height);
    AVCodecContext* pCodecContext{ nullptr };
private:
    AVFormatContext* pFormatContext{ nullptr };
    AVCodecParameters* codecParam{ nullptr };
    const AVCodec* pCodec{ nullptr };
    AVPacket* pPacket{ nullptr };
    AVFrame* pFrame{ nullptr };
    const std::string fileName;

};

