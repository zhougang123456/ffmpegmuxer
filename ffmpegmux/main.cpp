#include "ffmpegmux.hpp"
#include <iostream>
int main(int argc, char** argv) {
    std::cout << "ffmpeg muxer";
    FFmpegMuxer* muxer = new FFmpegMuxer();
    if (!muxer->init_video("mp4", AVMEDIA_TYPE_VIDEO, AV_CODEC_ID_H264)) {
		printf("Cannot init video muxer\n");

    }
	AVFormatContext* input_ctx = NULL;
	const char* in_file = "d://oceans.h264";
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