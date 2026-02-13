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
#include <libavutil/hwcontext_cuda.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <nvEncodeAPI.h>
#include <cuda.h>
#include <cudaGL.h>

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
    } OutputStream;

    VideoEncoder(const juce::String& file, int width, int height);
    
    int encode(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame, AVPacket* pkt);
    
    void addVideoFrame();

    bool startRecordingSession();
    
    bool finishRecordingSession();

    void cleanup();

    unsigned int getTextureID() {
		return texture_id;
    }

    bool isActive() {
        return active;
    }

private:

    bool initialiseVideo(OutputStream* ost, AVFormatContext* oc, const AVCodec** codec);
    void openVideo(AVFormatContext* oc, const AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg);
    AVFrame* allocFrame(enum AVPixelFormat pix_fmt, int width, int height);
    
    int getDeviceName(juce::String& gpuName) {
        //Setup the cuda context for hardware encoding with ffmpeg
        NV_ENC_BUFFER_FORMAT eFormat = NV_ENC_BUFFER_FORMAT_IYUV;
        int iGpu = 0;
        CUresult ret;
        int driverVersion;
        ret = cuDriverGetVersion(&driverVersion);
        if (ret == CUDA_ERROR_INVALID_VALUE) {
			DBG("No version of CUDA found!");
			return -1;
        }
        ret = cuInit(0);
        if (ret != CUDA_SUCCESS) {
            DBG("Cuda instance failed to initialise!");
            return -2;
        }
        int nGpu = 0;
        ret = cuDeviceGetCount(&nGpu);
        if (ret != CUDA_SUCCESS) {
            DBG("Cuda instance failed to cuDeviceGetCount!");
            return -3;
        }
        if (nGpu <= iGpu) {
            DBG("GPU ordinal out of range.");
            return -4;
        }
        CUdevice cuDevice = 0;
        ret = cuDeviceGet(&cuDevice, iGpu);
        if (ret != CUDA_SUCCESS) {
            DBG("Cuda instance failed to cuDeviceGet!");
            return -5;
        }
        char szDeviceName[80];
        ret = cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice);
        if (ret != CUDA_SUCCESS) {
            DBG("Cuda instance failed to cuDeviceGetName!");
            return -6;
        }
        gpuName = szDeviceName;
        return 1;
    }

    const juce::String file_name;
    int width, height;
    int have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;

    OutputStream video_st = { 0 };
    const AVOutputFormat* fmt;
    AVFormatContext* oc;
    const AVCodec* audio_codec, * video_codec;
    
    // Cuda and nvenc related variables.
    AVBufferRef* avBufferDevice, *avBufferFrame;
    CUcontext* cudaContext;
    unsigned int texture_id;
    CUDA_MEMCPY2D memcopyStruct;
    CUgraphicsResource cudaTextureResource;

    bool active;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VideoEncoder)

};