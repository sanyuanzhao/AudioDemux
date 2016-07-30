#include "audio_demux.h"

static int decode_packet(int *got_frame, int cached, AudioOperation* pnAudioOperation)
{     
    int ret = 0;
	int decoded = pnAudioOperation->pkt.size;
    *got_frame = 0;

    if (pnAudioOperation->pkt.stream_index == pnAudioOperation->audio_stream_idx) {
        /* decode audio frame */
        ret = avcodec_decode_audio4(pnAudioOperation->audio_dec_ctx, pnAudioOperation->frame, got_frame, &pnAudioOperation->pkt);
        if (ret < 0) {
            fprintf(stderr, "Error decoding audio frame (%s)\n", av_err2str(ret));
            return ret;
        }
        /* Some audio decoders decode only part of the packet, and have to be
         * called again with the remainder of the packet data.
         * Sample: fate-suite/lossless-audio/luckynight-partial.shn
         * Also, some decoders might over-read the packet. */
        decoded = FFMIN(ret, pnAudioOperation->pkt.size);

        if (*got_frame) {
			/*1、AVFrame中的nb_samples不是网上那些傻逼翻译成的采样率，而是采样值的个数，并且是指这一帧音频数据每个声道包含的采样值的个数。
			*2、av_get_bytes_per_sample获取的是每个采样值的字节数。因为采样值肯定是数字，既然是数字就需要用bit来表示。总之，该接口表示一个采样值包含了几个字节。
			*因此、unpadded_linesize表示每一帧音频数据的字节数。
			*/
			int bytes_per_sample = av_get_bytes_per_sample(pnAudioOperation->frame->format);
			size_t unpadded_linesize = pnAudioOperation->frame->nb_samples * bytes_per_sample;
			memcpy(pnAudioOperation->decoded_data + pnAudioOperation->decoded_data_length, pnAudioOperation->frame->extended_data[0], unpadded_linesize);
			pnAudioOperation->decoded_data_length += unpadded_linesize;
        }
    }
    return decoded;
}

static int open_codec_context(int *stream_idx,
                              AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodecContext *dec_ctx = NULL;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec_ctx = st->codec;
        dec = avcodec_find_decoder(dec_ctx->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int audio_read_package(AudioOperation* pnAudioOperation)
{
	int ret = -1;
	int got_frame = 0;
	pnAudioOperation->pkt.data = NULL;
	pnAudioOperation->pkt.size = 0;
	pnAudioOperation->decoded_data_length = 0;
	/* read frames from the file */
	if(av_read_frame(pnAudioOperation->fmt_ctx, &pnAudioOperation->pkt) >= 0) {
		AVPacket orig_pkt = pnAudioOperation->pkt;
		do {
			ret = decode_packet(&got_frame, 0, pnAudioOperation);
			if (ret < 0)
				break;
			pnAudioOperation->pkt.data += ret;
			pnAudioOperation->pkt.size -= ret;
		} while (pnAudioOperation->pkt.size > 0);
		av_packet_unref(&orig_pkt);
		return pnAudioOperation->decoded_data_length;
	}
	else
	{
		return AUDIO_DEMUX_ERROR;
	}
	
}

void audio_close_demux(AudioOperation* pnAudioOperation)
{
	if (pnAudioOperation != NULL)
	{
		avcodec_close(pnAudioOperation->audio_dec_ctx);
		avformat_close_input(&pnAudioOperation->fmt_ctx);
		av_frame_free(&pnAudioOperation->frame);
		if (pnAudioOperation->decoded_data != NULL) 
		{
			free(pnAudioOperation->decoded_data);
		}
		free(pnAudioOperation);
	}
}


int audio_open_demux(const char* url, AudioProperty* pnAudioProperty)
{
	AudioOperation* pnAudioOperation = NULL;
	int ret = 0, got_frame;
    
	pnAudioOperation = (AudioOperation*)malloc(sizeof(AudioOperation));
	memset((void*)pnAudioOperation, 0, sizeof(AudioOperation));
	pnAudioOperation->src_filename = url;

    /* register all formats and codecs */
    av_register_all();

    /* open input file, and allocate format context */
    if (avformat_open_input(&(pnAudioOperation->fmt_ctx), pnAudioOperation->src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", pnAudioOperation->src_filename);
		return AUDIO_DEMUX_ERROR;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(pnAudioOperation->fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
		return AUDIO_DEMUX_ERROR;
    }

    if (open_codec_context(&(pnAudioOperation->audio_stream_idx), pnAudioOperation->fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
		pnAudioOperation->audio_stream = pnAudioOperation->fmt_ctx->streams[pnAudioOperation->audio_stream_idx];
		pnAudioOperation->audio_dec_ctx = pnAudioOperation->audio_stream->codec;
    }


    if (!pnAudioOperation->audio_stream) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

	pnAudioOperation->frame = av_frame_alloc();
    if (!pnAudioOperation->frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&(pnAudioOperation->pkt));
	pnAudioOperation->pkt.data = NULL;
	pnAudioOperation->pkt.size = 0;
	pnAudioOperation->decoded_data = (uint8_t*)malloc(MAX_AUDIO_FRAME_SIZE);
    if (pnAudioOperation->audio_stream) {
        pnAudioProperty->nEncode = pnAudioOperation->audio_dec_ctx->sample_fmt;
		pnAudioProperty->nChannel = pnAudioOperation->audio_dec_ctx->channels;
		pnAudioProperty->nSample = pnAudioOperation->audio_dec_ctx->sample_rate;
		pnAudioProperty->nDuration = pnAudioOperation->audio_stream->duration 
			 /pnAudioOperation->audio_stream->time_base.den
			 *pnAudioOperation->audio_stream->time_base.num;
    }
	return (int)pnAudioOperation;

end:
	audio_close_demux(pnAudioOperation);
	return AUDIO_DEMUX_ERROR;
}
