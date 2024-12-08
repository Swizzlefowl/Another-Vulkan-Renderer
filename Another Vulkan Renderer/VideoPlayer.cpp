#include "VideoPlayer.hpp"
#include <fmt/core.h>
#include <iostream>
VideoPlayer::VideoPlayer(const std::string& name, avr::Context& other) :encoder{other} {
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

    rgbVideoFrame = av_frame_alloc();
    rgbVideoFrame->width = pCodecContext->width;
    rgbVideoFrame->height = pCodecContext->height;
    rgbVideoFrame->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(rgbVideoFrame, 0);
}

VideoPlayer::~VideoPlayer(){
    avformat_close_input(&pFormatContext);
    avformat_free_context(pFormatContext);
    avcodec_free_context(&pCodecContext);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&rgbVideoFrame);
}

std::span<uint8_t> VideoPlayer::getFrame(std::uint32_t& width, std::uint32_t& height){
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
    /*AVFrame* rgbVideoFrame = av_frame_alloc();
    rgbVideoFrame->width = pFrame->width;
    rgbVideoFrame->height = pFrame->height;
    rgbVideoFrame->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(rgbVideoFrame, 0);*/

    sws_scale(sws_ctx, pFrame->data,
        pFrame->linesize, 0, pCodecContext->height,
        rgbVideoFrame->data, rgbVideoFrame->linesize);

    size_t size = rgbVideoFrame->width * rgbVideoFrame->height * 4;
    std::span<uint8_t> frameData(rgbVideoFrame->data[0] ,size);

    //std::vector<uint8_t> frameData(size);
    //int index{};
    //for (auto& pixel : frameData) {
        //pixel = rgbVideoFrame->data[0][index];
        //index++;
    //}
    //av_frame_free(&rgbVideoFrame);
    return frameData;
}

VideoPlayer::Encoder::Encoder(avr::Context& other) : ctx{other} {
}

VideoPlayer::Encoder::~Encoder(){
    av_write_trailer(format_context);
    avio_closep(&format_context->pb);
}

void VideoPlayer::Encoder::open(const std::string& fileName, int width, int height){
    avformat_alloc_output_context2(&format_context, nullptr, nullptr, fileName.c_str());
    if (!format_context)
    {
        std::cout << "could not allocate output format" << std::endl;
        exit(-1);
    }

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        std::cout << "could not find encoder" << std::endl;
        exit(-1);
    }

    stream = avformat_new_stream(format_context, nullptr);
    if (!stream)
    {
        std::cout << "could not create stream" << std::endl;
        exit(-1);
    }
    stream->id = (int)(format_context->nb_streams - 1);

    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context)
    {
        std::cout << "could not allocate mContext codec context" << std::endl;
        exit(-1);
    }
    // appearently timebase is just frames per second??
    // in this case 1 frame per 60th of a second
    // frametime is reverse here
    // pts time here is just a monotonically incresing value
    // dts is the same
    // they mean presentaion time and decoding time
    // remember to write a header and before closing the
    // to write a trailer
    //codec_context->codec_id = format_context->oformat->video_codec;
    codec_context->bit_rate = 900000;
    codec_context->width = width;
    codec_context->height = height;
    stream->time_base = (AVRational){ 1, 60 };
    codec_context->time_base = stream->time_base;
    //codec_context->framerate = (AVRational){ 60, 1 };
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_context->gop_size = 5;
    codec_context->max_b_frames = 2;

    if (format_context->oformat->flags & AVFMT_GLOBALHEADER)
        codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    avcodec_parameters_from_context(stream->codecpar, codec_context);
    avcodec_open2(codec_context, codec, nullptr);
    //avformat_init_output(format_context, nullptr);
    avio_open(&format_context->pb, fileName.c_str(), AVIO_FLAG_WRITE);

    avformat_write_header(format_context, nullptr);
    frame = av_frame_alloc();
    packet = av_packet_alloc();

    frame->width = codec_context->width;
    frame->height = codec_context->height;
    frame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(frame, 0);
}

void VideoPlayer::Encoder::writeFrame(std::vector<uint8_t>& data){
    AVFrame* srcFrame = av_frame_alloc();
    srcFrame->width = codec_context->width;
    srcFrame->height = codec_context->height;
    srcFrame->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(srcFrame, 0);
    av_image_fill_arrays(srcFrame->data, srcFrame->linesize, data.data(), AV_PIX_FMT_RGBA, srcFrame->width, srcFrame->height, 1);

    auto sws_ctx = sws_getContext(srcFrame->width,
        srcFrame->height,
        AV_PIX_FMT_RGBA,
        codec_context->width,
        codec_context->height,
        AV_PIX_FMT_YUV420P,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );
    

    //av_frame_make_writable(rgbVideoFrame);
    sws_scale(sws_ctx, srcFrame->data,
        srcFrame->linesize, 0, srcFrame->height,
        frame->data, frame->linesize);

    frame->pts = frame_index;
    avcodec_send_frame(this->codec_context, frame);

    avcodec_receive_packet(codec_context, packet);

    packet->pts = frame_index;
    packet->dts = frame_index;
    av_packet_rescale_ts(packet, codec_context->time_base, stream->time_base);
    // packet->pts = temp->best_effort_timestamp;
     //packet->dts = temp->best_effort_timestamp;
    packet->stream_index = stream->index;
    av_interleaved_write_frame(format_context, packet);
    //stbi_write_png("out.png", rgbVideoFrame->width, rgbVideoFrame->height, STBI_rgb_alpha, rgbVideoFrame->data[0], rgbVideoFrame->linesize[0]);
    av_frame_free(&srcFrame);
    frame_index++;
}
