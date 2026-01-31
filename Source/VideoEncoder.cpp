/*
  ==============================================================================

    VideoEncoder.cpp
    Created: 23 Jan 2026 3:31:10pm
    Author:  lucas

  ==============================================================================
*/

#include "VideoEncoder.h"

VideoEncoder::VideoEncoder(const juce::String& file, const juce::String& codec, int width, int height) : file_name(file), codec_name(codec), width(width), height(height) {
    active = false;
}

bool VideoEncoder::startRecordingSession() {
    if (active)
        return false;

    int ret;

    codec = avcodec_find_encoder_by_name(codec_name.toRawUTF8()); // get the encoder by name;
    if (!codec) {
        DBG("Codec '%s' not found!\n", codec_name);
        return false;
    }

    videoContext = avcodec_alloc_context3(codec);
    if (!videoContext) {
        DBG("Could not allocate video codec context!\n");
        return false;
    }

    videoContext->pix_fmt = AV_PIX_FMT_YUV420P;
    videoContext->codec_id = codec->id;
    videoContext->codec_type = AVMEDIA_TYPE_VIDEO;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(videoContext->priv_data, "preset", "slow", 0);

    avformat_alloc_output_context2(&formatContext, nullptr, "mp4", file_name.toRawUTF8());
    if (!formatContext) {
        DBG("Could not create output context");
        return false;
    }
    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext->pb, file_name.toRawUTF8(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            DBG("Could not open output file via avio");
            if (formatContext->pb)
                avio_closep(&formatContext->pb);
            avcodec_free_context(&videoContext);
            avformat_free_context(formatContext);
            return false;
        }
    }

    stream = avformat_new_stream(formatContext, codec);
    if (!stream) {
        DBG("Could not allocate a codec stream!\n");
        if (!(formatContext->oformat->flags & AVFMT_NOFILE))
            avio_closep(&formatContext->pb);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    videoContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    videoContext->width = width;
    videoContext->height = height;
    videoContext->max_b_frames = 2;
    videoContext->profile = FF_PROFILE_H264_HIGH;
    videoContext->bit_rate = 480000;
    videoContext->time_base = av_make_q(1, 25); // compiler doesn't support compound literals, so this helper function is useful here.
    videoContext->framerate = av_make_q(25, 1);
    videoContext->gop_size = 12;
    stream->time_base = videoContext->time_base;

    if (codec->id == AV_CODEC_ID_H264) {
        av_opt_set(videoContext->priv_data, "rc", "vbr_hq", 0);
        av_opt_set(videoContext->priv_data, "cq", "19", 0);
    }
    ret = avcodec_open2(videoContext, codec, NULL);
    if (ret < 0) {
        DBG("Could not open codec: %s!\n", av_err2str(ret));
        if (!(formatContext->oformat->flags & AVFMT_NOFILE))
            avio_closep(&formatContext->pb);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    stream->codecpar->codec_tag = 0;
    stream->time_base = videoContext->time_base;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    avcodec_parameters_from_context(stream->codecpar, videoContext); // inform stream of audio context params.

    ret = avformat_write_header(formatContext, NULL);
    if (ret < 0) {
        DBG("Could not write stream header to the output file!");
        if (!(formatContext->oformat->flags & AVFMT_NOFILE))
            avio_closep(&formatContext->pb);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }
    
    frameIndex = 0;
    active = true;
    return true;
}

void VideoEncoder::addVideoFrame(const juce::Image& image) {
    if (!active)
        return;
    struct SwsContext* scalarContext = sws_getContext(image.getWidth(), image.getHeight(), AV_PIX_FMT_BGR0, videoContext->width, videoContext->height, videoContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
    if (!scalarContext) {
        DBG("Could not create scalar context!\n");
        return;
    }
    // all that needs to be done here is to just encode the frame.
    AVFrame* frame;
    frame = av_frame_alloc();
    if (!frame) {
        DBG("Could not allocate new video frame!\n");
        sws_freeContext(scalarContext);
        return;
    }

    frame->format = videoContext->pix_fmt;
    frame->width = videoContext->width;
    frame->height = videoContext->height;
    frame->pts = av_rescale_q(frameIndex++, videoContext->time_base, stream->time_base);
    
    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        DBG("Could not allocate the video frame data!\n");
        sws_freeContext(scalarContext);
        av_frame_free(&frame);
        return;
    }

    av_frame_make_writable(frame);

    juce::Image::BitmapData data(image, 0, 0, image.getWidth(), image.getHeight());
    uint8_t* source[4] = { data.data, nullptr, nullptr, nullptr };

    int srcStride[4] = { data.lineStride, 0, 0, 0 };
    sws_scale(scalarContext, source, srcStride, 0, image.getHeight(), frame->data, frame->linesize);

    encode(frame);

    sws_freeContext(scalarContext);
    av_frame_free(&frame);
}

void VideoEncoder::encode(AVFrame* frame) {
    if (!active)
        return;

    int ret = avcodec_send_frame(videoContext, frame);
    if (ret < 0) {
        DBG("Error during encoding!\n");
        return;
    }

    AVPacket* packet = av_packet_alloc();
    if (!packet)
        return;

    while (true) {
        ret = avcodec_receive_packet(videoContext, packet);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        if (ret < 0) {
            DBG("Encoding error");
            break;
        }

        packet->stream_index = stream->index;

        av_packet_rescale_ts(packet, videoContext->time_base, stream->time_base);

        ret = av_interleaved_write_frame(formatContext, packet);
        if (ret < 0) {
            DBG("Error during encoding! Couldn't mux frame.\n");
            break;
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
}

bool VideoEncoder::finishRecordingSession() {
    if (!active)
        return false;

    avcodec_send_frame(videoContext, nullptr);

    AVPacket* packet = av_packet_alloc();
    if (!packet)
        return false;

    int ret;
    while (true) {
        ret = avcodec_receive_packet(videoContext, packet);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        if (ret < 0) {
            DBG("Flush error\n");
            break;
        }

        packet->stream_index = stream->index;

        av_packet_rescale_ts(packet, videoContext->time_base, stream->time_base);

        av_interleaved_write_frame(formatContext, packet);
        av_packet_unref(packet);
    }

    av_packet_free(&packet);

    av_write_trailer(formatContext);
    
    // free memory
    if (!(formatContext->oformat->flags & AVFMT_NOFILE))
        avio_closep(&formatContext->pb);
    avcodec_free_context(&videoContext);
    avformat_free_context(formatContext);
    active = false;
    return true;
}