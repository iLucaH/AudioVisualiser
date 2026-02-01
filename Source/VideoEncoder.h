/*
  ==============================================================================

    VideoEncoder.h
    Created: 23 Jan 2026 3:31:10pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#define STREAM_PIX_FMT_DEFAULT AV_PIX_FMT_YUV420P
#define STREAM_FRAME_RATE 30

#define SCALE_FLAGS SWS_BICUBIC

class VideoEncoder {

public:

    // a wrapper around a single output AVStream - mux.c 2003
    typedef struct OutputStream {
        AVStream* st;
        AVCodecContext* enc;

        /* pts of the next frame that will be generated */
        int64_t next_pts;
        int samples_count;

        AVFrame* frame;
        AVFrame* tmp_frame;

        AVPacket* tmp_pkt;

        float t, tincr, tincr2;

        struct SwsContext* sws_ctx;
        struct SwrContext* swr_ctx;
    } OutputStream;

    VideoEncoder(const juce::String& file, const juce::String& codec, int width, int height);
    
    int encode(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame, AVPacket* pkt);
    
    void addVideoFrame(const juce::Image& image);

    bool startRecordingSession();
    
    bool finishRecordingSession();

    void fill_yuv_image(AVFrame* pict, int frame_index, int width, int height);

private:

    bool initialiseVideo(OutputStream* ost, AVFormatContext* oc, const AVCodec** codec, enum AVCodecID codec_id);
    void openVideo(AVFormatContext* oc, const AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg);
    AVFrame* allocFrame(enum AVPixelFormat pix_fmt, int width, int height);

    const juce::String file_name, codec_name;
    int width, height;
    int have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;

    OutputStream video_st = { 0 };
    const AVOutputFormat* fmt;
    AVFormatContext* oc;
    const AVCodec* audio_codec, * video_codec;

    bool active;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoEncoder)

};