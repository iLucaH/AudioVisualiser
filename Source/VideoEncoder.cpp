/*
  ==============================================================================

    VideoEncoder.cpp
    Created: 23 Jan 2026 3:31:10pm
    Author:  lucas

    Sources:
    * https://stackoverflow.com/questions/49862610/opengl-to-ffmpeg-encode
    * https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/mux.c
    * https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/encode_video.c
    * https://github.com/filmstro/filmstro_ffmpeg/blob/master/modules/filmstro_ffmpeg/filmstro_ffmpeg_FFmpegVideoWriter.cpp
    * https://docs.nvidia.com/video-technologies/video-codec-sdk/13.0/ffmpeg-with-nvidia-gpu/index.html
    * https://developer.nvidia.com/video-codec-sdk/download

  ==============================================================================
*/

#include "VideoEncoder.h"

#include "Texture.h"

VideoEncoder::VideoEncoder(const juce::String& file, int width, int height) : file_name(file), width(width), height(height), memcopyStruct({0}) {
    active = false;
    texture_id = create_gl_texture_id(width, height);
}

bool VideoEncoder::initialiseVideo(OutputStream* ost, AVFormatContext* oc, const AVCodec** codec) {
    AVCodecContext* codecContext;
    int i, ret;

    // Find the encoder using the codec_id discovered from the AVOutputFormat
    *codec = avcodec_find_encoder_by_name("h264_nvenc");
    if (!(*codec)) {
        DBG("Could not find encoder for '%s'\n",
            avcodec_get_name(codec_id));
        return false;
    }

    // Create an initial temp packet for the output stream struct.
    ost->tmp_pkt = av_packet_alloc();
    if (!ost->tmp_pkt) {
        DBG("Could not allocate AVPacket\n");
        return false;
    }

    // Create the stream (AVStream). NULL because that parameter does nothing.
    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        DBG("Could not allocate stream\n");
        return false;
    }

    // Not too sure just yet why this is necessary but it is present in mux.c line 151.
    ost->st->id = oc->nb_streams - 1;
    ost->next_pts = 0;

    // Create the codec context (AVCodecContext)
    codecContext = avcodec_alloc_context3(*codec);
    if (!codecContext) {
        DBG("Could not alloc an encoding context.");
        return false;
    }
    // Assign this new codec context to the output stream struct.
    ost->enc = codecContext;
    if ((*codec)->type == AVMEDIA_TYPE_VIDEO) {
        // Apply video settings to the codec context.
        codecContext->codec_id = (*codec)->id;
        codecContext->bit_rate = 400000;
        codecContext->width = width % 2 == 0 ? width : width - 1; // Must be a multiple of 2.
        codecContext->height = height % 2 == 0 ? height : height - 1; // Must be a multiple of 2.
        
        ost->st->time_base = av_make_q(1, STREAM_FRAME_RATE);
        codecContext->framerate = av_make_q(STREAM_FRAME_RATE, 1);
        codecContext->time_base = ost->st->time_base;

        // One intra frame every 12 frames. Must be set by the user.
        codecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
        codecContext->pix_fmt = AV_PIX_FMT_CUDA;
        // AV_PIX_FMT_RGBA is used instead of AV_PIX_FMT_YUV420P because we want the GPU to understand that the opengl data
        // is in RGB format and therefore should be converted from RGB to nvenc's output context format which is YUV.
        codecContext->sw_pix_fmt = AV_PIX_FMT_RGBA;

        // Inform the codecContext to seperate stream headers if the format requires it.
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
            codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    } else {
        // Not doing audio just yet.
        return false;
    }

    // Cuda related setup.
    juce::String gpuName;
    getDeviceName(gpuName);
    ret = av_hwdevice_ctx_create(&avBufferDevice, AV_HWDEVICE_TYPE_CUDA, NULL, NULL, 0);
    if (ret < 0) {
        DBG("Could not create a AV_HWDEVICE_TYPE_CUDA instance.");
        return false;
    }

    // Cast down to access cuda context.
    AVHWDeviceContext* hwDevContext = (AVHWDeviceContext*)(avBufferDevice->data);
    AVCUDADeviceContext* cudaDevCtx = (AVCUDADeviceContext*)(hwDevContext->hwctx);
    cudaContext = &(cudaDevCtx->cuda_ctx);

    // Create the hwframe_context.
    // This is an abstraction of a cuda buffer for us. This enables us to, with one call, setup the cuda buffer and ready it for input.
    avBufferFrame = av_hwframe_ctx_alloc(avBufferDevice);
    AVHWFramesContext* frameCtxPtr = (AVHWFramesContext*)(avBufferFrame->data);
    frameCtxPtr->width = width;
    frameCtxPtr->height = height;
    // AV_PIX_FMT_RGBA is used instead of AV_PIX_FMT_YUV420P because we want the GPU to understand that the opengl data
    // is in RGB format and therefore should be converted from RGB to nvenc's output context format which is YUV.
    frameCtxPtr->sw_format = AV_PIX_FMT_RGBA;
    frameCtxPtr->format = AV_PIX_FMT_CUDA;
    //frameCtxPtr->device_ref = avBufferDevice;
    //frameCtxPtr->device_ctx = (AVHWDeviceContext*)avBufferDevice->data;

    // Init the frame so that we can allocate a cuda buffer.
    ret = av_hwframe_ctx_init(avBufferFrame);
    if (ret < 0) {
        av_buffer_unref(&avBufferDevice);
        av_buffer_unref(&avBufferFrame);
        DBG("Could not init a av_hwframe_ctx_init frame.");
        return false;
    }

    // Cast the OGL texture/buffer to cuda ptr.
    CUresult res;
    CUcontext oldCtx; // Calls to oldCtx are allowed to fail.
    res = cuCtxPopCurrent(&oldCtx);
    res = cuCtxPushCurrent(*cudaContext);
    res = cuGraphicsGLRegisterImage(&cudaTextureResource, texture_id, juce::gl::GL_TEXTURE_2D, CU_GRAPHICS_REGISTER_FLAGS_READ_ONLY);
    if (res != CUDA_SUCCESS) {
        av_buffer_unref(&avBufferDevice);
        av_buffer_unref(&avBufferFrame);
        cuCtxDestroy(*cudaContext);
        DBG("Could not register a cuGraphicsGLRegisterImage gl image.");
    }
    res = cuCtxPopCurrent(&oldCtx);

    // Assign some hardware accel specific data to AvCodecContext.
    codecContext->hw_device_ctx = av_buffer_ref(avBufferDevice);
    codecContext->hw_frames_ctx = av_buffer_ref(avBufferFrame);

    // Setup some cuda stuff for memcpy-ing later
    memcopyStruct.srcXInBytes = 0;
    memcopyStruct.srcY = 0;
    memcopyStruct.srcMemoryType = CUmemorytype::CU_MEMORYTYPE_ARRAY;

    memcopyStruct.dstXInBytes = 0;
    memcopyStruct.dstY = 0;
    memcopyStruct.dstMemoryType = CUmemorytype::CU_MEMORYTYPE_DEVICE;

    return true;
}

AVFrame* VideoEncoder::allocFrame(enum AVPixelFormat pix_fmt, int width, int height) {
    AVFrame* frame;
    int ret;

    frame = av_frame_alloc();
    if (!frame)
        return NULL;

    //frame->format = pix_fmt; passing avBufferFrame will inform the frame of these details I believe.
    //frame->width = width;
    //frame->height = height;

    // Allocate RGB video frame buffer.
    ret = av_hwframe_get_buffer(avBufferFrame, frame, 0);

    //// Allocate buffers for the frame data.
    //ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        DBG("Could not allocate frame data.\n");
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
}

void VideoEncoder::openVideo(AVFormatContext* oc, const AVCodec* codec, OutputStream* ost, AVDictionary* opt_arg) {
    int ret;
    AVCodecContext* c = ost->enc;
    AVDictionary* opt = NULL;

    // Copy the dictionary settings over to the codec context.
    av_dict_copy(&opt, opt_arg, 0);

    // Add NVENC-specific options
    av_dict_set(&opt, "preset", "medium", 0);  // or "slow", "medium", "fast"
    av_dict_set(&opt, "tune", "hq", 0);
    av_dict_set(&opt, "rc", "vbr", 0);

    // Open the codec.
    ret = avcodec_open2(c, codec, &opt);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DBG("Could not open the video codec: " << errbuf << " (error code: " << ret << ")");
        av_dict_free(&opt);
        return;
    }

    // The dictionary settings were used to open the codec, and are no longer needed.
    av_dict_free(&opt);

    /* allocate and init a reusable frame */
    ost->frame = allocFrame(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        DBG("Could not allocate video frame\n");
        return;
    }

    ost->tmp_frame = nullptr;

    // Inform the muxer of the stream. All muxing function calls such as av_write_header
    // will be aware of the stream and codec parameters. 
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        DBG("Could not copy the stream parameters\n");
        return;
    }
}

bool VideoEncoder::startRecordingSession() {
    if (active)
        return false;
    DBG("Starting Recording Session!");
    int ret, i;
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

    // Init stream and codec.
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        // do add video stream here
        initialiseVideo(&video_st, oc, &video_codec);
        have_video = 1;
        encode_video = 1;
    }

    if (have_video)
        openVideo(oc, video_codec, &video_st, opt);
    // Confirm the video context actually opened.
    if (!video_st.enc || !avcodec_is_open(video_st.enc)) {
        DBG("Failed to open video openVideo(oc, video_codec, &video_st, opt);.");
        cleanup();
        return false;
    }

    // Analyse the file for debug printing.
    av_dump_format(oc, 0, file_name.toRawUTF8(), 1);

    // Open the output file if it hasn't already been opened. Perhaps not needed for future code cleanup
    // since it will be closed onRecordingEnd. Assertion should be made there.
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, file_name.toRawUTF8(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            DBG("Could not open the output file.\n");
            return false;
        }
    }

    // Write the stream header.
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        DBG("Could not write a header to the output file.\n");
        return false;
    }
    active = true;
    return true;
}

void VideoEncoder::addVideoFrame() {
    if (!active) return;

    OutputStream* ost = &video_st;

    if (av_frame_make_writable(ost->frame) < 0)
        return;

    //Perform cuda mem copy for input buffer
    CUresult cuRes;
    CUarray mappedArray;
    CUcontext oldCtx;

    //Get context
    cuRes = cuCtxPopCurrent(&oldCtx); // THIS IS ALLOWED TO FAIL
    cuRes = cuCtxPushCurrent(*cudaContext);

    //Get Texture
    cuRes = cuGraphicsResourceSetMapFlags(cudaTextureResource, CU_GRAPHICS_MAP_RESOURCE_FLAGS_READ_ONLY);
    cuRes = cuGraphicsMapResources(1, &cudaTextureResource, 0);

    //Map texture to cuda array
    cuRes = cuGraphicsSubResourceGetMappedArray(&mappedArray, cudaTextureResource, 0, 0); // Nvidia says its good practice to remap each iteration as OGL can move things around

    //Release texture
    cuRes = cuGraphicsUnmapResources(1, &cudaTextureResource, 0);

    //Setup for memcopy
    memcopyStruct.srcArray = mappedArray;
    memcopyStruct.dstDevice = (CUdeviceptr)ost->frame->data[0]; // Make sure to copy devptr as it could change, upon resize
    memcopyStruct.dstPitch = ost->frame->linesize[0];   // Linesize is generated by hwframe_context
    memcopyStruct.WidthInBytes = ost->frame->width * 4; //* 4 needed for each pixel
    memcopyStruct.Height = ost->frame->height;          //Vanilla height for frame

    //Do memcpy
    cuRes = cuMemcpy2D(&memcopyStruct);

    //release context
    cuRes = cuCtxPopCurrent(&oldCtx);

    ost->frame->pts = ost->next_pts++;

    encode(oc, ost->enc, ost->st, ost->frame, ost->tmp_pkt);
}

int VideoEncoder::encode(AVFormatContext* fmt_ctx, AVCodecContext* c, AVStream* st, AVFrame* frame, AVPacket* pkt) {
    if (!active)
        return 1;

    int ret;

    // Send the frame to the encoder.
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        return 1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            ("Error encoding a frame");
            return 1;
        }

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, c->time_base, st->time_base);
        pkt->stream_index = st->index;

        // Write the compressed frame to the media file.
        ret = av_interleaved_write_frame(fmt_ctx, pkt);

        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
        if (ret < 0) {
            DBG("Error while writing output packet");
            return 1;
        }
    }

    return ret == AVERROR_EOF ? 1 : 0;

}

bool VideoEncoder::finishRecordingSession() {
    if (!active)
        return false;

    cleanup();
    active = false;
    DBG("Finishing Recording Session!");
    return true;
}

void VideoEncoder::cleanup() {
    // If the encoder is not active, then there should be nothing to clean up.
    if (!active)
        return;

    // Flush the encoder by parsing a nullptr.
    encode(oc, video_st.enc, video_st.st, nullptr, video_st.tmp_pkt);

    av_write_trailer(oc);

    // Free memory
    if (have_video) {
        OutputStream* ost = &video_st;
        avcodec_free_context(&ost->enc);
        av_frame_free(&ost->frame);
        av_frame_free(&ost->tmp_frame);
        av_packet_free(&ost->tmp_pkt);

        av_buffer_unref(&avBufferDevice);
        av_buffer_unref(&avBufferFrame);
        cuGraphicsUnregisterResource(cudaTextureResource);
    }

    // Close the file if it's still open.
    if (!(fmt->flags & AVFMT_NOFILE))
        avio_closep(&oc->pb);

    // Free the stream attached to the output context.
    avformat_free_context(oc);
}