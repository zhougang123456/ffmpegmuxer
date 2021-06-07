extern "C" {
#include "libavformat/avformat.h"
}
class FFmpegMuxer
{
public:
	FFmpegMuxer();
	~FFmpegMuxer();
    bool init_audio(const char* mux_type, AVMediaType media_type, AVCodecID code_id);
    bool mux_audio(uint8_t* data, int size);
    bool init_video(const char* mux_type, AVMediaType media_type, AVCodecID code_id);
    bool mux_video(uint8_t* data, int size);
private:
    static int write_packet(void* opaque, uint8_t* buffer, int size);
    static int write_video_packet(void* opaque, uint8_t* buffer, int size);

    AVFormatContext* fmt_ctx_;
    AVIOContext* avio_ctx_;
    AVStream* av_stream_;
    AVPacket* pkt_;
    int64_t timestamp_;
};
