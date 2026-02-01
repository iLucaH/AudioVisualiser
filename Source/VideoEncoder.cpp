/*
  ==============================================================================

    VideoEncoder.cpp
    Created: 23 Jan 2026 3:31:10pm
    Author:  lucas

  ==============================================================================
*/

#include "VideoEncoder.h"

#define STREAM_PIX_FMT_DEFAULT AV_PIX_FMT_YUV420P
#define STREAM_FRAME_RATE 30

#define SCALE_FLAGS SWS_BICUBIC

VideoEncoder::VideoEncoder(const juce::String& file, const juce::String& codec, int width, int height) : file_name(file), codec_name(codec), width(width), height(height) {
    active = false;
}

bool VideoEncoder::initialiseVideo(OutputStream* ost, AVFormatContext* oc, const AVCodec** codec, enum AVCodecID codec_id) {
    AVCodecContext* codecContext;
    int i;

    // Find the encoder using the codec_id discovered from the AVOutputFormat
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
            avcodec_get_name(codec_id));
        return false;
    }

    // Create an initial temp packet for the output stream struct.
    ost->tmp_pkt = av_packet_alloc();
    if (!ost->tmp_pkt) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        return false;
    }

    // Create the stream (AVStream). NULL because that parameter does nothing.
    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not allocate stream\n");
        return false;
    }

    // Not too sure just yet why this is necessary but it is present in mux.c line 151.
    ost->st->id = oc->nb_streams - 1;

    // Create the codec context (AVCodecContext)
    codecContext = avcodec_alloc_context3(*codec);
    if (!codecContext) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        return false;
    }
    // Assign this new codec context to the output stream struct.
    ost->enc = codecContext;
    if ((*codec)->type == AVMEDIA_TYPE_VIDEO) {
        // Apply video settings to the codec context.
        codecContext->codec_id = codec_id;
        codecContext->bit_rate = 400000;
        codecContext->width = width % 2 == 0 ? width : width - 1; // Must be a multiple of 2.
        codecContext->height = height % 2 == 0 ? height : height - 1; // Must be a multiple of 2.
        
        ost->st->time_base = av_make_q(1, STREAM_FRAME_RATE);
        codecContext->framerate = av_make_q(STREAM_FRAME_RATE, 1);
        codecContext->time_base = ost->st->time_base;

        // One intra frame every 12 frames. Must be set by the user.
        codecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
        codecContext->pix_fmt = STREAM_PIX_FMT_DEFAULT;

        // Inform the codecContext to seperate stream headers if the format requires it.
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
            codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    } else {
        // Not doing audio just yet.
        return false;
    }

    return true;
}

bool VideoEncoder::startRecordingSession() {
    if (active)
        return false;
    active = true;
    int ret, i;
    int have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;
    AVDictionary* opt = NULL;
    
    // Set options here. AVDictionary is a key value data structure for ffmpeg. 
    // It is used for options in this instance.
    // 
    // av_dict_set(&opt, argv[i] + 1, argv[i + 1], 0);

    // Allocate the output media context (AVFormatContext and AVOutputFormat).
    avformat_alloc_output_context2(&oc, NULL, NULL, file_name.toRawUTF8());
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", file_name.toRawUTF8());
    }
    if (!oc)
        return 1;

    fmt = oc->oformat;

    /* Add the audio and video streams using the default format codecs
 * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        // do add video stream here

        have_video = 1;
        encode_video = 1;
    }

    return true;
}

void VideoEncoder::addVideoFrame(const juce::Image& image) {
    if (!active)
        return;
}

void VideoEncoder::encode(AVFrame* frame) {
    if (!active)
        return;

}

bool VideoEncoder::finishRecordingSession() {
    if (!active)
        return false;
    active = false;
    return true;
}