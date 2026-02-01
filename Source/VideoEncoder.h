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
    
    void encode(AVFrame* frame);
    
    void addVideoFrame(const juce::Image& image);

    bool startRecordingSession();
    
    bool finishRecordingSession();

private:

    bool initialiseVideo(OutputStream* ost, AVFormatContext* oc, const AVCodec** codec, enum AVCodecID codec_id);

    const juce::String file_name, codec_name;
    int width, height;

    OutputStream video_st = { 0 }, audio_st = { 0 };
    const AVOutputFormat* fmt;
    AVFormatContext* oc;
    const AVCodec* audio_codec, * video_codec;

    bool active;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoEncoder)

};