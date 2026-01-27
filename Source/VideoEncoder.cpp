/*
  ==============================================================================

    VideoEncoder.cpp
    Created: 23 Jan 2026 3:31:10pm
    Author:  lucas

  ==============================================================================
*/

#include "VideoEncoder.h"

VideoEncoder::VideoEncoder(juce::String& file, juce::String& codec, int width, int height) : file_name(file), codec_name(codec), width(width), height(height) {

}

bool VideoEncoder::startRecordingSession() {
    int ret;

    codec = avcodec_find_encoder_by_name(codec_name.toRawUTF8()); // get the encoder by name;
    if (!codec) {
        DBG("Codec '%s' not found!\n", codec_name);
        av_free(&codec);
        return false;
    }


    videoContext = avcodec_alloc_context3(codec);
    if (!videoContext) {
        DBG("Could not allocate video codec context!\n");
        av_free(&codec);
        avcodec_free_context(&videoContext);
        return false;
    }

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(videoContext->priv_data, "preset", "slow", 0);

    formatContext = avformat_alloc_context();
    outputFormat = av_guess_format("mp4", nullptr, nullptr);
    if (outputFormat) {
        formatContext->oformat = outputFormat;
    } else {
        DBG("Could not find mp4 output format context!");
        av_free(&codec);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    stream = avformat_new_stream(formatContext, codec);
    if (!stream) {
        DBG("Could not allocate a codec stream!\n");
        av_free(&codec);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    videoContext->width = width;
    videoContext->height = height;
    videoContext->bit_rate = 480000;
    videoContext->time_base = av_make_q(1, 25); // compiler doesn't support compound literals, so this helper function is useful here.
    videoContext->framerate = av_make_q(25, 1);
    avcodec_parameters_from_context(stream->codecpar, videoContext); // inform stream of audio context params.

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(videoContext->priv_data, "preset", "slow", 0);

    ret = avcodec_open2(videoContext, codec, NULL);
    if (ret < 0) {
        DBG("Could not open codec: %s!\n", av_err2str(ret));
        av_free(&codec);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    f = fopen(file_name.toRawUTF8(), "wb");
    if (!f) {
        DBG("Could not open %s!\n", file_name);
        av_free(&codec);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    ret = avformat_write_header(formatContext, NULL);
    if (ret < 0) {
        DBG("Could not write stream header to the output file!");
        av_free(&codec);
        avcodec_free_context(&videoContext);
        avformat_free_context(formatContext);
        return false;
    }

    return true;
}

void VideoEncoder::addVideoFrame(const juce::Image& image) {
    struct SwsContext* scalarContext = sws_getContext(image.getWidth(), image.getHeight(), AV_PIX_FMT_YUV420P, videoContext->width, videoContext->height, videoContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
    // all that needs to be done here is to just encode the frame.
    AVFrame* frame;
    frame = av_frame_alloc();
    if (!frame) {
        DBG("Could not allocate new video frame!\n");
        return;
    }

    frame->format = videoContext->pix_fmt;
    frame->width = videoContext->width;
    frame->height = videoContext->height;
    
    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        DBG("Could not allocate the video frame data!\n");
        return;
    }

    juce::Image::BitmapData data(image, 0, 0, image.getWidth(), image.getHeight());
    uint8_t* source[4] = { data.data, nullptr, nullptr, nullptr };

    sws_scale(scalarContext, source, &data.lineStride, 0, image.getHeight(), frame->data, frame->linesize);

    encode(frame);

    sws_freeContext(scalarContext);
    av_frame_free(&frame);
}

void VideoEncoder::encode(AVFrame* frame) {
    int ret;
    
    AVPacket* packet;
    packet = av_packet_alloc();
    if (!packet)
        return;

    ret = avcodec_send_frame(videoContext, frame);
    if (ret < 0) {
        DBG("Error sending a frame for encoding!\n");
        av_packet_free(&packet);
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(videoContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            av_packet_free(&packet);
            return;
        }

        fwrite(packet->data, 1, packet->size, f);
        av_packet_unref(packet);
    }

    av_packet_free(&packet);
}

bool VideoEncoder::finishRecordingSession() {
    // may still need to mux // ffmpeg -f h264 -i test.mp4 -c copy output.mp4
    // free memory
}