extern "C" {
#include <libavformat/avformat.h>
};
#include <iostream>
static void dump_opus(uint8_t* outbuf, int size) 
{
   
    static FILE* file = NULL;
    if (file == NULL)
    {
        char Buf[128];
        sprintf(Buf, "D:\\output.opus");
        file = fopen(Buf, "wb");

    }
    if (file != NULL) {
        fwrite(outbuf, size, 1, file);
    }
}
static void dump_h264(uint8_t* outbuf, int size)
{

    static FILE* file = NULL;
    if (file == NULL)
    {
        char Buf[128];
        sprintf(Buf, "D:\\output.h264");
        file = fopen(Buf, "wb");

    }
    if (file != NULL) {
        fwrite(outbuf, size, 1, file);
    }
}

typedef struct Frame {
    unsigned char* buffer;
    int size;
}Frame;

int read_packet(void* opaque, uint8_t* buf, int buf_size) {
    
    Frame* frame = (Frame*)opaque;
    if (frame->buffer != NULL) {
        memcpy(buf, frame->buffer, buf_size);
        return buf_size;
    }
    return 0;
}

void demuxaudio()
{
    static FILE* file = NULL;
    if (file == NULL) {
        file = fopen("D:\\output.webm", "rb+");
    }
    else {
        return;
    }
    int data_size;
    unsigned char* pBmpBuf = NULL;
    if (file) {
        if (fseek(file, 0, SEEK_END))
            return;
        data_size = ftell(file);
        if (fseek(file, 0, 0))  //跳过BITMAPFILEHEADE
        {
            return;
        }
        pBmpBuf = new unsigned char[data_size];
        fread(pBmpBuf, sizeof(unsigned char), data_size, file);
        fclose(file);
    }
    
    Frame* frame = new Frame();

    frame->buffer = pBmpBuf;
    frame->size = 0;
    AVFormatContext* fmt_ctx = avformat_alloc_context();
    int buffer_size = 32768;
    unsigned char* buffer = (unsigned char*)av_malloc(buffer_size);
    AVIOContext* avio_ctx = avio_alloc_context(buffer, buffer_size, 0, frame, read_packet, NULL, NULL);
    fmt_ctx->pb = avio_ctx;
    fmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
    int ret = avformat_open_input(&fmt_ctx, "", NULL, NULL);
    if (ret < 0) {
        std::cout << "open file failed!" << std::endl;
        return;
    }
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        std::cout << "read stream info failed!" << std::endl;
        return;
    }
    int audioindex = -1;
    audioindex = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    std::cout << "audio index: " << audioindex << std::endl;
   
    AVPacket* pkt = av_packet_alloc();
    frame->buffer = pBmpBuf;
    while (av_read_frame(fmt_ctx, pkt) == 0)
    {
        if (pkt->stream_index == audioindex) {
            dump_opus(pkt->data, pkt->size);
            //std::cout << "audio size: " << pkt->size << std::endl;

        }
        else {
            std::cout << "audio size: " << pkt->size << std::endl;
        }
        av_packet_unref(pkt);
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
    }
    

    avio_context_free(&avio_ctx);
    if (pkt) {
        av_packet_free(&pkt);
    }
}

void demuxvideo()
{
    static FILE* file = NULL;
    if (file == NULL) {
        file = fopen("D:\\output.mp4", "rb+");
    }
    else {
        return;
    }
    int data_size;
    unsigned char* pBmpBuf = NULL;
    if (file) {
        if (fseek(file, 0, SEEK_END))
            return;
        data_size = ftell(file);
        if (fseek(file, 0, 0))  //跳过BITMAPFILEHEADE
        {
            return;
        }
        pBmpBuf = new unsigned char[data_size];
        fread(pBmpBuf, sizeof(unsigned char), data_size, file);
        fclose(file);
    }

    Frame* frame = new Frame();

    frame->buffer = pBmpBuf;
    frame->size = 0;
    AVFormatContext* fmt_ctx = avformat_alloc_context();
    int buffer_size = 32768;
    unsigned char* buffer = (unsigned char*)av_malloc(buffer_size);
    AVIOContext* avio_ctx = avio_alloc_context(buffer, buffer_size, 0, frame, read_packet, NULL, NULL);
    fmt_ctx->pb = avio_ctx;
    fmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
    int ret = avformat_open_input(&fmt_ctx, "", NULL, NULL);
    if (ret < 0) {
        std::cout << "open file failed!" << std::endl;
        return;
    }
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        std::cout << "read stream info failed!" << std::endl;
        return;
    }
    int videoindex = -1;
    videoindex = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    std::cout << "video index: " << videoindex << std::endl;

    AVPacket* pkt = av_packet_alloc();
    frame->buffer = pBmpBuf;
    while (av_read_frame(fmt_ctx, pkt) == 0)
    {
        if (pkt->stream_index == videoindex) {
            dump_h264(pkt->data, pkt->size);
            //std::cout << "audio size: " << pkt->size << std::endl;

        }
        else {
            std::cout << "audio size: " << pkt->size << std::endl;
        }
        av_packet_unref(pkt);
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
    }


    avio_context_free(&avio_ctx);
    if (pkt) {
        av_packet_free(&pkt);
    }

}
 int write_buffer(void* opaque, uint8_t* buf, int buf_size) {
    
    static FILE* fp_write = NULL;
    if (fp_write == NULL) {
        fp_write = fopen("output.mp4", "wb+");
    }
    if (fp_write) {
        fwrite(buf, 1, buf_size, fp_write);
    }

    return buf_size;
}

void muxaudio()
{   

    static FILE* file = NULL;
    unsigned char* pBmpBuf = NULL;
    if (file == NULL) {
        file = fopen("D:\\output.opus", "rb+");
    }
    else {
        return;
    }
    int data_size = 0;
    if (file) {
        if (fseek(file, 0, SEEK_END))
            return;
        data_size = ftell(file);
        if (fseek(file, 0, 0))  //跳过BITMAPFILEHEADE
        {
            return;
        }

        pBmpBuf = new unsigned char[data_size];
        fread(pBmpBuf, sizeof(unsigned char), data_size, file);
    }
    fclose(file);
    const char* format_name = "webm";
    const char* file_name = "output.webm";
    AVFormatContext* fmt_ctx;
    int ret = avformat_alloc_output_context2(&fmt_ctx, NULL, format_name, NULL);
    if (ret < 0) {
        std::cout << "alloc output context failed!" << std::endl;
        return;
    }
    
    AVStream* out_stream = avformat_new_stream(fmt_ctx, NULL);
    out_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    out_stream->codecpar->codec_id = AV_CODEC_ID_OPUS;
    out_stream->codecpar->channels = 2;
    out_stream->codecpar->sample_rate = 48000;

    int buffer_size = 32768;
    unsigned char* outbuffer = (unsigned char*)av_malloc(buffer_size);
    AVIOContext* avio_out = avio_alloc_context(outbuffer, buffer_size, AVIO_FLAG_WRITE | AVIO_FLAG_NONBLOCK, NULL, NULL, write_buffer, NULL);
    fmt_ctx->pb = avio_out;
    fmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_FLUSH_PACKETS;
    
    if ((ret = avformat_write_header(fmt_ctx, NULL)) < 0) {
        std::cout << "write header failed!" << std::endl;
        return;
    }
    AVPacket* pkt = av_packet_alloc();
    pkt->stream_index = 0;
    pkt->pts = 0;
    pkt->dts = 0;
    pkt->duration = 2000;
    pkt->data = pBmpBuf;
    pkt->size = data_size;

    ret = av_write_frame(fmt_ctx, pkt);


    av_write_trailer(fmt_ctx);
    avio_context_free(&avio_out);
    avformat_free_context(fmt_ctx);
}

static const uint8_t h264_extradata[] = {
   0x01, 0x4d, 0x40, 0x1e, 0xff, 0xe1, 0x00, 0x02, 0x67, 0x4d, 0x01, 0x00, 0x02, 0x68, 0xef};

void muxvideo()
{

    static FILE* file = NULL;
    unsigned char* pBmpBuf = NULL;
    if (file == NULL) {
        file = fopen("D:\\test.h264", "rb+");
    }
    else {
        return;
    }
    int data_size = 0;
    if (file) {
        if (fseek(file, 0, SEEK_END))
            return;
        data_size = ftell(file);
        if (fseek(file, 0, 0))  //跳过BITMAPFILEHEADE
        {
            return;
        }

        pBmpBuf = new unsigned char[data_size];
        fread(pBmpBuf, sizeof(unsigned char), data_size, file);
    }
    fclose(file);
    const char* format_name = "mp4";
    AVFormatContext* fmt_ctx;
    int ret = avformat_alloc_output_context2(&fmt_ctx, NULL, format_name, NULL);
    if (ret < 0) {
        std::cout << "alloc output context failed!" << std::endl;
        return;
    }
    
    AVStream* out_stream = avformat_new_stream(fmt_ctx, NULL);
   
    out_stream->codecpar->extradata_size = sizeof(h264_extradata);
    out_stream->codecpar->extradata = (uint8_t*)av_malloc(out_stream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(out_stream->codecpar->extradata, h264_extradata, sizeof(h264_extradata));
    out_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    out_stream->codecpar->codec_id = AV_CODEC_ID_H264;
    out_stream->codecpar->codec_tag = 0;
    out_stream->codecpar->width = 960;
    out_stream->codecpar->height = 400;
    out_stream->codecpar->format = AV_PIX_FMT_YUV420P;
  
    out_stream->codecpar->sample_aspect_ratio.den = 1;
    out_stream->codecpar->sample_aspect_ratio.num = 1;
    int buffer_size = 32768;
    unsigned char* outbuffer = (unsigned char*)av_malloc(buffer_size);
    AVIOContext* avio_out = avio_alloc_context(outbuffer, buffer_size, AVIO_FLAG_WRITE | AVIO_FLAG_NONBLOCK, NULL, NULL, write_buffer, NULL);
    fmt_ctx->pb = avio_out;
    fmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_FLUSH_PACKETS;

    AVDictionary* opts = NULL;
    av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+faststart", 0);
    av_dict_set_int(&opts, "flush_packets", 1, 0);
    if ((ret = avformat_write_header(fmt_ctx, &opts)) < 0) {
        std::cout << "write header failed!" << std::endl;
        av_dict_free(&opts);
        return;
    }
    av_dict_free(&opts);

    AVPacket* pkt = av_packet_alloc();
    pkt->stream_index = 0;
    pkt->pts = 0;
    pkt->dts = 0;
    pkt->data = pBmpBuf;
    pkt->size = data_size;
    ret = av_write_frame(fmt_ctx, pkt);
    ret = av_write_frame(fmt_ctx, NULL);
    av_write_trailer(fmt_ctx);
    avio_context_free(&avio_out);
    avformat_free_context(fmt_ctx);
}

int main(int argc, char** argv)
{
    //demuxaudio();
    //muxaudio();
    demuxvideo();
    //muxvideo();
    return 0;
}