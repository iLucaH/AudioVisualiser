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

VideoEncoder::VideoEncoder(const juce::String& file, const juce::String& codec, int width, int height) : file_name(file), codec_name(codec), width(width), height(height) {
    active = false;
}

int VideoEncoder::getDeviceName(juce::String& gpuName) {
    //Setup the cuda context for hardware encoding with ffmpeg
    NV_ENC_BUFFER_FORMAT eFormat = NV_ENC_BUFFER_FORMAT_IYUV;
    int iGpu = 0;
    CUresult ret;
    ret = cuInit(0);
    if (ret != CUDA_SUCCESS) {
        DBG("Cuda instance failed to initialise!");
        return -1;
    }
    int nGpu = 0;
    ret = cuDeviceGetCount(&nGpu);
    if (ret != CUDA_SUCCESS) {
        DBG("Cuda instance failed to cuDeviceGetCount!");
        return -1;
    }
    if (nGpu <= iGpu) {
        DBG("GPU ordinal out of range.");
        return -1;
    }
    CUdevice cuDevice = 0;
    ret = cuDeviceGet(&cuDevice, iGpu);
    if (ret != CUDA_SUCCESS) {
        DBG("Cuda instance failed to cuDeviceGet!");
        return -1;
    }
    char szDeviceName[80];
    ret = cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice);
    if (ret != CUDA_SUCCESS) {
        DBG("Cuda instance failed to cuDeviceGetName!");
        return -1;
    }
    gpuName = szDeviceName;
    return 1;
}

bool VideoEncoder::initialiseVideo(OutputStream* ost, AVFormatContext* oc, const AVCodec** codec, enum AVCodecID codec_id) {
    AVCodecContext* codecContext;
    int i;

    // Find the encoder using the codec_id discovered from the AVOutputFormat
    *codec = avcodec_find_encoder(codec_id);
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

    // Create the codec context (AVCodecContext)
    codecContext = avcodec_alloc_context3(*codec);
    if (!codecContext) {
        DBG("Could not alloc an encoding context\n");
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

AVFrame* VideoEncoder::allocFrame(enum AVPixelFormat pix_fmt, int width, int height) {
    AVFrame* frame;
    int ret;

    frame = av_frame_alloc();
    if (!frame)
        return NULL;

    frame->format = pix_fmt;
    frame->width = width;
    frame->height = height;

    // Allocate buffers for the frame data.
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        DBG("Could not allocate frame data.\n");
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

    // Open the codec.
    ret = avcodec_open2(c, codec, &opt);
    if (ret < 0) {
        DBG("Could not open the video codec.");
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

    // If the pixel format isn't YUV420P, then we need to convert to it, and we can use
    // a temp frame to hold the data and wait for sws_scale to be called on it which will
    // transfer the output format to the right format.
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = allocFrame(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            DBG("Could not allocate temporary video frame\n");
            return;
        }
    }

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
    active = true;
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
        initialiseVideo(&video_st, oc, &video_codec, fmt->video_codec);
        have_video = 1;
        encode_video = 1;
    }

    if (have_video)
        openVideo(oc, video_codec, &video_st, opt);

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
    return true;
}

void VideoEncoder::addVideoFrame(const juce::Image& image)
{
    if (!active) return;

    OutputStream* ost = &video_st;

    if (av_frame_make_writable(ost->frame) < 0)
        return;

    juce::Image::BitmapData data(image, juce::Image::BitmapData::readOnly);

    uint8_t* srcData[4] = { data.data, nullptr, nullptr, nullptr };
    int srcLinesize[4] = { data.lineStride, 0, 0, 0 };

    if (!ost->sws_ctx) {
        ost->sws_ctx = sws_getContext(
            image.getWidth(), image.getHeight(),
            AV_PIX_FMT_BGRA,
            ost->enc->width, ost->enc->height,
            ost->enc->pix_fmt,
            SCALE_FLAGS, nullptr, nullptr, nullptr
        );
    }

    sws_scale(
        ost->sws_ctx,
        srcData,
        srcLinesize,
        0,
        image.getHeight(),
        ost->frame->data,
        ost->frame->linesize
    );

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

/* Prepare a dummy image. 
 *  Copyright (c) 2003 Fabrice Bellard
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
void VideoEncoder::fill_yuv_image(AVFrame* pict, int frame_index,
    int width, int height)
{
    int x, y, i;

    i = frame_index;

    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++) {
        for (x = 0; x < width / 2; x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}

bool VideoEncoder::finishRecordingSession() {
    if (!active)
        return false;

    active = false;

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
        sws_freeContext(ost->sws_ctx);
        // swr_free(&ost->swr_ctx);
    }

    // Close the file if it's still open.
    if (!(fmt->flags & AVFMT_NOFILE))
        avio_closep(&oc->pb);

    // Free the stream attached to the output context.
    avformat_free_context(oc);

    return true;
}