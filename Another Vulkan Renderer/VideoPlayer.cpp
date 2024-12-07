#include "VideoPlayer.hpp"
#include <fmt/core.h>

VideoPlayer::VideoPlayer(const std::string& name){
    pFormatContext = avformat_alloc_context();
    if (auto res = avformat_open_input(&pFormatContext, name.c_str(), NULL, NULL) != 0) {
        fmt::println("couldnt open file");
        throw std::runtime_error("failed to open video file");
    }
    avformat_find_stream_info(pFormatContext, NULL);

    
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        AVCodecParameters* pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        const AVCodec* pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (!pLocalCodec)
            break;
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            codecParam = pLocalCodecParameters;
            printf("Codec %s ID %d bit_rate %lld \n", pLocalCodec->long_name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
            printf("Video Codec: resolution %d x %d \n", pLocalCodecParameters->width, pLocalCodecParameters->height);
            break;
        }
        else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            fmt::println("audio codec");
        }

    }

    pCodec = avcodec_find_decoder(codecParam->codec_id);
    pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, codecParam);
    avcodec_open2(pCodecContext, pCodec, NULL);

    pPacket = av_packet_alloc();
    pFrame = av_frame_alloc();
}

VideoPlayer::~VideoPlayer(){
    avformat_free_context(pFormatContext);
    avcodec_free_context(&pCodecContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
}

std::vector<uint8_t> VideoPlayer::getFrame(std::uint32_t& width, std::uint32_t& height){
    while (true) {
        auto result = av_read_frame(pFormatContext, pPacket);
        if (result != 0)
            throw std::runtime_error("video ended");
        avcodec_send_packet(pCodecContext, pPacket);
        avcodec_receive_frame(pCodecContext, pFrame);
        if (pFrame->width > 0 || pFrame->height > 0)
            break;
        else
            continue;
       
        //auto name = fmt::format("test{}.png", i);
        //stbi_write_png(name.c_str(), pCodecContext->width, pCodecContext->height, STBI_rgb_alpha, rgbVideoFrame->data[0], rgbVideoFrame->width * 4);
    }

    auto sws_ctx = sws_getContext(pCodecContext->width,
        pCodecContext->height,
        pCodecContext->pix_fmt,
        pCodecContext->width,
        pCodecContext->height,
        AV_PIX_FMT_RGBA,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );
    AVFrame* rgbVideoFrame = av_frame_alloc();
    rgbVideoFrame->width = pFrame->width;
    rgbVideoFrame->height = pFrame->height;
    rgbVideoFrame->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(rgbVideoFrame, 0);

    sws_scale(sws_ctx, pFrame->data,
        pFrame->linesize, 0, pCodecContext->height,
        rgbVideoFrame->data, rgbVideoFrame->linesize);

    size_t size = rgbVideoFrame->width * rgbVideoFrame->height * 4;
    std::vector<uint8_t> frameData(size);
    int index{};
    for (auto& pixel : frameData) {
        pixel = rgbVideoFrame->data[0][index];
        index++;
    }
    av_frame_free(&rgbVideoFrame);
    return frameData;
}
