#include "ffmpegmux.hpp"
#include <iostream>

static void dump_raw(uint8_t* outbuf, int size)
{
	static FILE* file = NULL;
	if (file == NULL)
	{
		char Buf[128];
		sprintf(Buf, "D:\\output.vp9");
		file = fopen(Buf, "wb");
	}
	if (file != NULL) {
		fwrite(outbuf, size, 1, file);
	}


}
int main(int argc, char** argv) {
    std::cout << "ffmpeg muxer";
    FFmpegMuxer* muxer = new FFmpegMuxer();
    if (!muxer->init_video("webm", AVMEDIA_TYPE_VIDEO, AV_CODEC_ID_VP9)) {
		printf("Cannot init video muxer\n");

    }
	AVFormatContext* input_ctx = NULL;
	//const char* in_file = "d://test.h265";
	const char* in_file = "d://vp9.webm";
	int video_stream = 0, ret;
	AVPacket packet;

	/* open the input file */
	if (avformat_open_input(&input_ctx, in_file, NULL, NULL) != 0) {
		printf("Cannot open in the input file\n");
		return -1;
	}

	/* find the video stream information */
	ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (ret < 0) {
		printf("Cannot find a video stream in the input file\n");
		return -1;
	}

	while (ret >= 0)
	{
		if ((ret = av_read_frame(input_ctx, &packet)) < 0)
		{
			break;
		}

		if (video_stream == packet.stream_index)
		{	
			printf(" frame size:%d \n", packet.size);
			dump_raw(packet.data, packet.size);
			muxer->mux_video(packet.data, packet.size);
			
		}
		else {
			printf(" frame size:%d \n", packet.size);
		}

		av_packet_unref(&packet);
	}


	avformat_close_input(&input_ctx);
    return 0;
}