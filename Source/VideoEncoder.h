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
}

class VideoEncoder {

public:

    VideoEncoder(const char* file, juce::String codec, int width, int height) : file_name(file), codec_name(codec), width(width), height(height) {
   }
    
    void encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt, FILE* outfile) {
        int ret;

        ret = avcodec_send_frame(enc_ctx, frame);
        if (ret < 0) {
            fprintf(stderr, "Error sending a frame for encoding\n");
            //exit(1);
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(enc_ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            else if (ret < 0) {
                fprintf(stderr, "Error during encoding\n");
                //exit(1);
            }

            fwrite(pkt->data, 1, pkt->size, outfile);
            av_packet_unref(pkt);
        }

        // ffmpeg -f h264 -i test.mp4 -c copy output.mp4
    }

    void test() {
        const AVCodec* codec;
        AVCodecContext* codecContext = NULL;
        AVFormatContext* formatContext = nullptr;
        int i, ret, x, y;
        FILE* f;
        AVFrame* frame;
        AVPacket* pkt;
        uint8_t endcode[] = { 0, 0, 1, 0xb7 };

        /* find the mpeg1video encoder */
        codec = avcodec_find_encoder_by_name(codec_name.toRawUTF8());
        if (!codec) {
            DBG("Codec '%s' not found\n", codec_name);
            //exit(1);
        }

        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            DBG("Could not allocate video codec context\n");
            //exit(1);
        }

        formatContext = avformat_alloc_context();
        const AVOutputFormat* oformat = av_guess_format("mp4", nullptr, nullptr);
        if (oformat) {
            formatContext->oformat = oformat;
        }

        pkt = av_packet_alloc();
        if (!pkt)
            //exit(1);

        /* put sample parameters */
        codecContext->bit_rate = 400000;
        /* resolution must be a multiple of two */
        codecContext->width = width;
        codecContext->height = height;
        /* frames per second */
        codecContext->time_base = av_make_q(1, 25); // compiler doesn't support compound literals, so this helper function is useful here.
        codecContext->framerate = av_make_q(25, 1);

        /* emit one intra frame every ten frames
         * check frame pict_type before passing frame
         * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
         * then gop_size is ignored and the output of encoder
         * will always be I frame irrespective to gop_size
         */
        codecContext->gop_size = 10;
        codecContext->max_b_frames = 1;
        codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

        if (codec->id == AV_CODEC_ID_H264)
            av_opt_set(codecContext->priv_data, "preset", "slow", 0);

        /* open it */
        ret = avcodec_open2(codecContext, codec, NULL);
        if (ret < 0) {
            DBG("Could not open codec: %s\n", av_err2str(ret));
            //exit(1);
        }

        f = fopen(file_name, "wb");
        if (!f) {
            DBG("Could not open %s\n", file_name);
            //exit(1);
        }

        frame = av_frame_alloc();
        if (!frame) {
            DBG("Could not allocate video frame\n");
            //exit(1);
        }
        frame->format = codecContext->pix_fmt;
        frame->width = codecContext->width;
        frame->height = codecContext->height;

        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            DBG("Could not allocate the video frame data\n");
            //exit(1);
        }

        /* encode 3 seconds of video */
        for (i = 0; i < 75; i++) {
            /* Make sure the frame data is writable.
               On the first round, the frame is fresh from av_frame_get_buffer()
               and therefore we know it is writable.
               But on the next rounds, encode() will have called
               avcodec_send_frame(), and the codec may have kept a reference to
               the frame in its internal structures, that makes the frame
               unwritable.
               av_frame_make_writable() checks that and allocates a new buffer
               for the frame only if necessary.
             */
            ret = av_frame_make_writable(frame);
            if (ret < 0)
                //exit(1);

            /* Prepare a dummy image.
               In real code, this is where you would have your own logic for
               filling the frame. FFmpeg does not care what you put in the
               frame.
             */
             /* Y */
            for (y = 0; y < codecContext->height; y++) {
                for (x = 0; x < codecContext->width; x++) {
                    frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
                }
            }

            /* Cb and Cr */
            for (y = 0; y < codecContext->height / 2; y++) {
                for (x = 0; x < codecContext->width / 2; x++) {
                    frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                    frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
                }
            }

            frame->pts = i;

            /* encode the image */
            encode(codecContext, frame, pkt, f);
        }

        /* flush the encoder */
        encode(codecContext, NULL, pkt, f);

        /* Add sequence end code to have a real MPEG file.
           It makes only sense because this tiny examples writes packets
           directly. This is called "elementary stream" and only works for some
           codecs. To create a valid file, you usually need to write packets
           into a proper file format or protocol; see mux.c.
         */
        if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
            fwrite(endcode, 1, sizeof(endcode), f);
        fclose(f);

        avcodec_free_context(&codecContext);
        avformat_free_context(formatContext);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }
    private:
        const char* file_name;
        juce::String codec_name;

        int width, height;
};