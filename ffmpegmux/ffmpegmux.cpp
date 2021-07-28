#include "ffmpegmux.hpp"
#include <iostream>
FFmpegMuxer::FFmpegMuxer()
{
}

FFmpegMuxer::~FFmpegMuxer()
{
}

static void dump_webm(uint8_t* outbuf, int size)
{
    static FILE* file = NULL;
    if (file == NULL)
    {
        char Buf[128];
        sprintf(Buf, "D:\\output.webm");
        file = fopen(Buf, "wb");
    }
    if (file != NULL) {
        fwrite(outbuf, size, 1, file);
    }
}

static void dump_mp4(uint8_t* outbuf, int size)
{
    static FILE* file = NULL;
    if (file == NULL)
    {
        char Buf[128];
        sprintf(Buf, "D:\\output.mp4");
        file = fopen(Buf, "wb");
    }
    if (file != NULL) {
        fwrite(outbuf, size, 1, file);
    }

}
bool FFmpegMuxer::init_audio(const char* mux_type, AVMediaType media_type, AVCodecID code_id)
{   
    int ret = avformat_alloc_output_context2(&fmt_ctx_, NULL, mux_type, NULL);
    if (ret < 0) {
        std::cout << "alloc avformat failed!";
        return false;
    }
    av_stream_ = avformat_new_stream(fmt_ctx_, NULL);
    av_stream_->codecpar->codec_type = media_type;
    av_stream_->codecpar->codec_id = code_id;
    av_stream_->codecpar->channels = 2;
    av_stream_->codecpar->sample_rate = 48000;
    
    int buffer_size = 32767;
    unsigned char* out_buffer = (unsigned char* )av_malloc(buffer_size);
    avio_ctx_ = avio_alloc_context(out_buffer, buffer_size, AVIO_FLAG_WRITE, NULL, NULL, write_packet, NULL);
    fmt_ctx_->pb = avio_ctx_;
    fmt_ctx_->flags = AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_FLUSH_PACKETS;
    pkt_ = av_packet_alloc();
    avformat_write_header(fmt_ctx_, NULL);
    return true;
}

static const uint8_t h264_extradata[] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1e, 0xf8, 0x0f, 0x00, 0x43, 0xbe, 0x8,
                                          0x00, 0x00, 0x00, 0x01, 0x68, 0xce, 0x38, 0x80 };

static const uint8_t h265_extradata[] = { 0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00, 0x03,
                                          0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x78, 0x95, 0x98, 0x09, 0x00,
                                          0x00, 0x00, 0x01, 0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 
                                          0x03, 0x00, 0x00, 0x03, 0x00, 0x78, 0xa0, 0x07, 0x82, 0x01, 0x91, 0x65, 0x95, 0x9a, 0x49,
                                          0x32, 0xbc, 0x04, 0x04, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x03, 0x01, 0x90, 0x20,  
                                          0x00, 0x00, 0x00, 0x01, 0x44, 0x01, 0xc1, 0x72, 0xb4, 0x62, 0x40};


bool FFmpegMuxer::init_video(const char* mux_type, AVMediaType media_type, AVCodecID code_id)
{   
    int ret = avformat_alloc_output_context2(&fmt_ctx_, NULL, mux_type, NULL);
    if (ret < 0) {
        std::cout << "alloc avformat failed!";
        return false;
    }
    fmt_ctx_->oformat->video_codec = code_id;
    av_stream_ = avformat_new_stream(fmt_ctx_, NULL);
    av_stream_->codecpar->codec_type = media_type;
    av_stream_->codecpar->codec_id = code_id;
    av_stream_->codecpar->width = 1280;
    av_stream_->codecpar->height = 720;
    av_stream_->codecpar->sample_aspect_ratio.den = 1;
    av_stream_->codecpar->sample_aspect_ratio.num = 1;
    av_stream_->codecpar->codec_tag = 0;
    AVDictionary* opts = NULL;

    switch (code_id)
    {
    case AV_CODEC_ID_H264: 
    {
        av_stream_->codecpar->extradata_size = sizeof(h264_extradata);
        av_stream_->codecpar->extradata = (uint8_t*)av_malloc(av_stream_->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(av_stream_->codecpar->extradata, h264_extradata, sizeof(h264_extradata));
        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
        av_dict_set(&opts, "brand", "mp42", 0);
        break;
    }
    case AV_CODEC_ID_HEVC:
    {   
        av_stream_->codecpar->extradata_size = sizeof(h265_extradata);
        av_stream_->codecpar->extradata = (uint8_t*)av_malloc(av_stream_->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(av_stream_->codecpar->extradata, h265_extradata, sizeof(h265_extradata));
        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
        av_dict_set(&opts, "brand", "mp42", 0);
        break;
    }
    default:
        break;
    }

    int buffer_size = 32767;
    unsigned char* outbuffer = (unsigned char*)av_malloc(buffer_size);
    AVIOContext* avio_ctx = avio_alloc_context(outbuffer, buffer_size, AVIO_FLAG_WRITE, NULL, NULL, write_video_packet, NULL);
    fmt_ctx_->pb = avio_ctx;
    fmt_ctx_->flags = AVFMT_FLAG_CUSTOM_IO;
    pkt_ = av_packet_alloc();
   
    avformat_write_header(fmt_ctx_, &opts);
    av_dict_free(&opts);
  
    return true;
}

bool FFmpegMuxer::mux_audio(uint8_t* data, int size)
{  
    int scale = av_stream_->time_base.den / 1000;
    pkt_->stream_index = 0;
    pkt_->pts = timestamp_;
    pkt_->dts = timestamp_;
    pkt_->duration = 10 * scale;
    timestamp_ += pkt_->duration;
    pkt_->data = data;
    pkt_->size = size;
    if (av_write_frame(fmt_ctx_, pkt_) < 0) {
        std::cout << "av write frame failed!";
    }
    av_write_frame(fmt_ctx_, NULL);
    av_packet_unref(pkt_);
    return true;
}

bool FFmpegMuxer::mux_video(uint8_t* data, int size)
{   
    int scale = av_stream_->time_base.den / 1000;
    pkt_->stream_index = 0;
    pkt_->pts = timestamp_;
    pkt_->dts = timestamp_;
    pkt_->duration = 40 * scale;
    timestamp_ += pkt_->duration;
    pkt_->data = data;
    pkt_->size = size;
    if (av_write_frame(fmt_ctx_, pkt_) < 0) {
        std::cout << "av write frame failed!";
    }
    av_write_frame(fmt_ctx_, NULL);
    av_packet_unref(pkt_);
    return true;
}

int FFmpegMuxer::write_packet(void* opaque, uint8_t* buffer, int size)
{
    dump_webm(buffer, size);
    return size;
}

 int FFmpegMuxer::write_video_packet(void* opaque, uint8_t * buffer, int size)
 {  
    printf(" packet size:%d \n", size);
    dump_webm(buffer, size);
    return size;
 }
