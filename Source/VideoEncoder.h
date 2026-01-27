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

    VideoEncoder(juce::String& file, juce::String& codec, int width, int height);
    ~VideoEncoder();
    
    void encode(AVFrame* frame);
    
    void addVideoFrame(const juce::Image& image);

    bool startRecordingSession();
    
    bool finishRecordingSession();

    private:

        juce::String file_name, codec_name;
        int width, height;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoEncoder)

private:
    FILE* f;
    AVFormatContext* formatContext;
    AVCodecContext* videoContext;
    const AVOutputFormat* outputFormat;
    const AVCodec* codec;
    AVStream* stream;
};