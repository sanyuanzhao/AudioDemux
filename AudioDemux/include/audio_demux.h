#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#define AUDIO_DEMUX_SUCCESS  (0)
#define AUDIO_DEMUX_ERROR    (-1)
#define MAX_AUDIO_FRAME_SIZE (192000) 

typedef struct AudioProperty
{
	int nSample;
	int nChannel;
	int nEncode;
	int64_t nDuration;
} AudioProperty;

typedef struct AudioOperation{
	AVFormatContext*fmt_ctx;
	AVCodecContext *audio_dec_ctx;
	enum AVPixelFormat pix_fmt;
	int audio_stream_idx;
	AVStream* audio_stream;
	const char *src_filename;
	AVFrame *frame;
	AVPacket pkt;
	int refcount;
	uint8_t* decoded_data;
	int decoded_data_length;
}AudioOperation;

int audio_open_demux(const char* url, AudioProperty* pnAudioProperty);
void audio_close_demux(AudioOperation* pnAudioOperation);
int audio_read_package(AudioOperation* pnAudioOperation);

#ifdef __cplusplus
}
#endif