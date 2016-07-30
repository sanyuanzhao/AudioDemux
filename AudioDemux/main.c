// Demux.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "audio_demux.h"
#include "demux.h"

int main()
{
	FILE* audio_dst_file = NULL;
	int sessionId = 0;
	AudioProperty oAudioProperty = { 0 };
	uint8_t* data = (uint8_t*)malloc(MAX_AUDIO_FRAME_SIZE);
	sessionId = audio_open_demux("hao.mp3", &oAudioProperty);
	printf("sessionId : %d\n", sessionId);
	if (sessionId > 0)
	{
		AudioOperation* oAudioOperation = (AudioOperation*)sessionId;
		audio_dst_file = fopen("hao.pcm", "wb");
		int read = 0;
		do
		{
			read =audio_read_package(oAudioOperation);
			printf("read : %d\n", oAudioOperation->decoded_data_length);
			if (audio_dst_file != NULL)
			{
				fwrite(oAudioOperation->decoded_data, read, 1, audio_dst_file);
			}
		} while (read >= 0);
		if (audio_dst_file != NULL)
		{
			fclose(audio_dst_file);
		}
		audio_close_demux(oAudioOperation);
	}
	/*
	char* argv[4] = { "hao.mp3", "hao.mp3", "hao.pcm", "hao.pcm" };
	demux(4, argv);
	*/
	return 0;
}

