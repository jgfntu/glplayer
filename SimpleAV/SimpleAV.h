/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifndef __SIMPLEAV_H__DEFINED__
#define __SIMPLEAV_H__DEFINED__

#ifdef __cplusplus
extern "C" {
#endif

#include "SAMutex.h"
#include "SAQueue.h"

#include <SDL/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

#define  SAABUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE) * 3 / 2)

typedef struct SAVideoPacket {
     AVFrame *frame_ptr;
     float pts;
} SAVideoPacket;

typedef struct SAAudioPacket {
     uint8_t *abuffer;
     int len;
     float pts;
} SAAudioPacket;

typedef struct SAContext {
     char *filename;

     SAMutex aq_lock_real, vpq_lock_real, apq_lock_real, packet_lock_real;
     
     SAMutex *aq_lock;
     SAQContext *aq_ctx;
     SAMutex *vpq_lock, *apq_lock;
     SAQContext *vpq_ctx, *apq_ctx;
     SAMutex *packet_lock;
     
     AVPacket pkt_temp;
     
     AVFormatContext *avfmt_ctx_ptr;
     AVCodecContext *a_codec_ctx, *v_codec_ctx;
     AVStream *audio_st, *video_st;
     AVCodec *a_codec, *v_codec;
     int v_stream, a_stream;

     AVFrame *frame;
     
     int v_width, v_height;
     int audio_eof, video_eof;
     float video_clock, a_clock;
} SAContext;

int SA_init(void);

SAContext *SA_open(char *);

void SA_close(SAContext *);

SAAudioPacket *SA_get_ap(SAContext *);

SAVideoPacket *SA_get_vp(SAContext *);

void SA_free_ap(SAAudioPacket *);

void SA_free_vp(SAVideoPacket *);

int SA_seek(SAContext *, float, float);

int _SA_read_packet(SAContext *);

int _SA_get_buffer(AVCodecContext *, AVFrame *);

void _SA_release_buffer(AVCodecContext *, AVFrame *);

int SA_get_width(SAContext *);

int SA_get_height(SAContext *);

float SA_get_duration(SAContext *);

// FIXME: maybe we should never use this sh*t?
// float SA_get_clock(void);

#ifdef __cplusplus
}
#endif

#endif
