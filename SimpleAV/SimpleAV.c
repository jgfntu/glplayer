/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "SAMutex.h"
#include "SAQueue.h"

#include "SimpleAV.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

#include <stdio.h>
#include <stdlib.h>

#define  TRUE     1
#define  FALSE    0

int SA_init(void)
{
     // av_log_set_flags(???); // FIXME: inplement this
     av_register_all();
     return 0; // FIXME
}

SAContext *SA_open(char *filename)
{
     int v_stream = -1, a_stream = -1, i;
     AVFormatContext *avfmt_ctx_ptr = NULL;
     AVCodecContext *v_codec_ctx = NULL, *a_codec_ctx = NULL;
     AVCodec *v_codec = NULL, *a_codec = NULL;

     /* allocating space for the context and temporary frame */
     SAContext *ctx_p = (SAContext *) malloc(sizeof(SAContext));
     if(ctx_p == NULL)
          goto OPEN_FAIL;
     memset(ctx_p, 0, sizeof(SAContext));
     ctx_p->vpq_ctx = ctx_p->apq_ctx = ctx_p->aq_ctx = NULL;
     ctx_p->avfmt_ctx_ptr = NULL;
     ctx_p->audio_eof = ctx_p->video_eof = FALSE;
     
     /* init the queue for storing decoder's output */
     ctx_p->filename = filename;
     ctx_p->vpq_ctx = SAQ_init();
     ctx_p->apq_ctx = SAQ_init();
     ctx_p->aq_ctx = SAQ_init();
     if(ctx_p->vpq_ctx == NULL || ctx_p->apq_ctx == NULL || ctx_p->aq_ctx == NULL)
          goto OPEN_FAIL;

     /* opening the file */
     if(avformat_open_input(&avfmt_ctx_ptr, filename, NULL, NULL) != 0)
          goto OPEN_FAIL;
     ctx_p->avfmt_ctx_ptr = avfmt_ctx_ptr;
     if(av_find_stream_info(avfmt_ctx_ptr) < 0)
          goto OPEN_FAIL;

     /* FIXME: do this for debugging. */
     av_dump_format(avfmt_ctx_ptr, 0, filename, 0);
     /* getting the video stream */
     for(i = 0; i < avfmt_ctx_ptr->nb_streams; i++)
          if(avfmt_ctx_ptr->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
               break;
     if(i == avfmt_ctx_ptr->nb_streams)
          goto OPEN_FAIL;
     ctx_p->v_stream = v_stream = i;

     /* getting the audio stream */
     for(i = 0; i < avfmt_ctx_ptr->nb_streams; i++)
          if(avfmt_ctx_ptr->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
               break;
     if(i == avfmt_ctx_ptr->nb_streams)
          goto OPEN_FAIL;
     ctx_p->a_stream = a_stream = i;

     ctx_p->v_codec_ctx = v_codec_ctx = avfmt_ctx_ptr->streams[v_stream]->codec;
     ctx_p->a_codec_ctx = a_codec_ctx = avfmt_ctx_ptr->streams[a_stream]->codec;
     
     /* set our userdata for calculating PTS */
     v_codec_ctx->get_buffer = _SA_get_buffer;
     v_codec_ctx->release_buffer = _SA_release_buffer;
     v_codec_ctx->opaque = (uint64_t *)malloc(sizeof(uint64_t));
     if(v_codec_ctx->opaque == NULL)
     {
          fprintf(stderr, "failed to allocate space for v_codec_ctx->opaque!\n");
          goto OPEN_FAIL;
     }
     *(uint64_t *)(v_codec_ctx->opaque) = AV_NOPTS_VALUE;

     /* getting the codec */
     ctx_p->v_codec = v_codec = avcodec_find_decoder(v_codec_ctx->codec_id);
     ctx_p->a_codec = a_codec = avcodec_find_decoder(a_codec_ctx->codec_id);
     if(v_codec == NULL || a_codec == NULL)
     {
          fprintf(stderr, "Unsupported codec!\n");
          goto OPEN_FAIL;
     }

     /* set audio_st and video_st */
     ctx_p->audio_st = avfmt_ctx_ptr->streams[a_stream];
     ctx_p->video_st = avfmt_ctx_ptr->streams[v_stream];
     
     /* FIXME: downmix. a BAD dirty hack. */
     ctx_p->a_codec_ctx->request_channels = FFMIN(2, ctx_p->a_codec_ctx->channels);

     if(avcodec_open(v_codec_ctx, v_codec) < 0 ||
        avcodec_open(a_codec_ctx, a_codec) < 0)
     {
          fprintf(stderr, "avcodec_open failed!\n");
          goto OPEN_FAIL;
     }
     
     /* setting other useful variables */
     ctx_p->v_width = ctx_p->v_codec_ctx->width;
     ctx_p->v_height = ctx_p->v_codec_ctx->height;
     ctx_p->video_clock = 0.0f;
     ctx_p->a_clock = 0.0f;

     /* for storing decoded frame. */
     ctx_p->frame = avcodec_alloc_frame();
     if(ctx_p->frame == NULL) {
          fprintf(stderr, "could not alloc an empty AVFrame!\n");
          goto OPEN_FAIL;
     }

     /* create the mutex needed to lock _SA_decode_packet(). */
     int aq_lock_inited = FALSE, vpq_lock_inited = FALSE,
          apq_lock_inited = FALSE, packet_lock_inited = FALSE;
     ctx_p->aq_lock = &(ctx_p->aq_lock_real);
     ctx_p->vpq_lock = &(ctx_p->vpq_lock_real);
     ctx_p->apq_lock = &(ctx_p->apq_lock_real);
     ctx_p->packet_lock = &(ctx_p->packet_lock_real);
     
     if(SAMutex_init(ctx_p->aq_lock) == 0)
          aq_lock_inited = TRUE;
     else
          ctx_p->aq_lock = NULL;
     
     if(SAMutex_init(ctx_p->vpq_lock) == 0)
          vpq_lock_inited = TRUE;
     else
          ctx_p->vpq_lock = NULL;
     
     if(SAMutex_init(ctx_p->apq_lock) == 0)
          apq_lock_inited = TRUE;
     else
          ctx_p->apq_lock = NULL;
     
     if(SAMutex_init(ctx_p->packet_lock) == 0)
          packet_lock_inited = TRUE;
     else
          ctx_p->packet_lock = NULL;
          
     if(aq_lock_inited == FALSE || vpq_lock_inited == FALSE ||
        apq_lock_inited == FALSE || packet_lock_inited == FALSE)
     {
          fprintf(stderr, "failed to create mutexes.\n");
          goto OPEN_FAIL;
     }

     /* evenything is ok. return the context. */
     return ctx_p;
     
OPEN_FAIL:
     SA_close(ctx_p);
     return NULL;
}

void SA_close(SAContext *sa_ctx)
{
     SAMutex_lock(sa_ctx->vpq_lock);
     SAMutex_lock(sa_ctx->apq_lock);
     SAMutex_lock(sa_ctx->aq_lock);
     
     if(sa_ctx == NULL)
          return;
     if(sa_ctx->v_codec_ctx != NULL)
     {
          if(sa_ctx->v_codec_ctx->opaque != NULL)
               free(sa_ctx->v_codec_ctx->opaque);
          avcodec_close(sa_ctx->v_codec_ctx);
     }
     if(sa_ctx->a_codec_ctx != NULL)
          avcodec_close(sa_ctx->a_codec_ctx);
     if(sa_ctx->avfmt_ctx_ptr != NULL)
          av_close_input_file(sa_ctx->avfmt_ctx_ptr);

     void *ptr;
     if(sa_ctx->vpq_ctx != NULL)
     {
          while((ptr = SAQ_pop(sa_ctx->vpq_ctx)) != NULL)
          {
               av_free_packet((AVPacket *)ptr);
               av_free(ptr);
          }
          free(sa_ctx->vpq_ctx);
     }
     if(sa_ctx->apq_ctx != NULL)
     {
          while((ptr = SAQ_pop(sa_ctx->apq_ctx)) != NULL)
          {
               av_free_packet((AVPacket *)ptr);
               av_free(ptr);
          }
          free(sa_ctx->apq_ctx);
     }
     if(sa_ctx->aq_ctx != NULL)
     {
          while((ptr = SAQ_pop(sa_ctx->aq_ctx)) != NULL)
               SA_free_ap(ptr);
          free(sa_ctx->aq_ctx);
     }

     if(sa_ctx->vpq_lock != NULL)
          SAMutex_destroy(sa_ctx->vpq_lock);
     if(sa_ctx->apq_lock != NULL)
          SAMutex_destroy(sa_ctx->apq_lock);
     if(sa_ctx->aq_lock != NULL)
          SAMutex_destroy(sa_ctx->aq_lock);
     if(sa_ctx->packet_lock != NULL)
          SAMutex_destroy(sa_ctx->packet_lock);

     if(sa_ctx->frame != NULL)
          av_free(sa_ctx->frame);
     
     free(sa_ctx);
     return;
}

int _SA_read_packet(SAContext *sa_ctx)
{
     SAMutex_lock(sa_ctx->packet_lock);
     
     AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
     av_init_packet(packet);
     if(av_read_frame(sa_ctx->avfmt_ctx_ptr, packet) < 0)
     {
          av_free(packet);
          
          SAMutex_unlock(sa_ctx->packet_lock);
          return -1;
     }

     if(av_dup_packet(packet) != 0)
     {
          fprintf(stderr, "av_dup_packet failed?\n");
          av_free_packet(packet);
          av_free(packet);
          
          SAMutex_unlock(sa_ctx->packet_lock);
          return -1;
     }
     
     if(packet->stream_index == sa_ctx->v_stream)
     {
          SAMutex_lock(sa_ctx->vpq_lock);
          SAQ_push(sa_ctx->vpq_ctx, packet);
          SAMutex_unlock(sa_ctx->vpq_lock);
     } else if(packet->stream_index == sa_ctx->a_stream)
     {
          SAMutex_lock(sa_ctx->apq_lock);
          SAQ_push(sa_ctx->apq_ctx, packet);
          SAMutex_unlock(sa_ctx->apq_lock);
     }

     SAMutex_unlock(sa_ctx->packet_lock);
     return 0;
}

SAVideoPacket *SA_get_vp(SAContext *sa_ctx)
{
     SAVideoPacket *ret;
     int frame_finished = 0;
     AVPacket *packet = NULL;
     AVFrame *v_frame = sa_ctx->frame;
     uint64_t last_dts;
     while(!frame_finished)
     {
          while(packet == NULL)
          {
               SAMutex_lock(sa_ctx->vpq_lock);
               packet = SAQ_pop(sa_ctx->vpq_ctx);
               SAMutex_unlock(sa_ctx->vpq_lock);
               if(packet == NULL)
                    if(_SA_read_packet(sa_ctx) < 0)
                         return NULL;
          }

          // FIXME: is using this without calling avcodec_alloc_frame() ok?
          *(uint64_t *)(sa_ctx->v_codec_ctx->opaque) = packet->pts;
          
          if(avcodec_decode_video2(sa_ctx->v_codec_ctx, v_frame,
                                   &frame_finished, packet) <= 0) {
               // FIXME: handle this in another way.
               fprintf(stderr, "Error while decoding video!\n");
          }

          last_dts = packet->dts;
          av_free_packet(packet);
          av_free(packet);
          packet = NULL;
     }
          
     ret = malloc(sizeof(SAVideoPacket));
     if(ret == NULL) {
          fprintf(stderr, "malloc failed\n");
          return NULL;
     }
     ret->frame_ptr = v_frame;

     uint64_t t_pts;
     if(last_dts != AV_NOPTS_VALUE)
          t_pts = last_dts;
     else if(v_frame->opaque != NULL &&
             *(uint64_t *)(v_frame->opaque) != AV_NOPTS_VALUE)
          t_pts = *(uint64_t *)(v_frame->opaque);
     else
          t_pts = 0;
               
     if(t_pts != 0)
          ret->pts = t_pts * av_q2d(sa_ctx->video_st->time_base);
     else
          ret->pts = sa_ctx->video_clock;
               
     float frame_delay = av_q2d(sa_ctx->video_st->codec->time_base);
     frame_delay += v_frame->repeat_pict * (frame_delay * 0.5);
     sa_ctx->video_clock = ret->pts + frame_delay;

     return ret;
}

SAAudioPacket *SA_get_ap(SAContext *sa_ctx)
{
     SAMutex_lock(sa_ctx->aq_lock);
     SAAudioPacket *ret = SAQ_pop(sa_ctx->aq_ctx);
     SAMutex_unlock(sa_ctx->aq_lock);

     if(ret != NULL)
     {
          printf("out: 1\n");
          return ret;
     }

     int n = 2 * sa_ctx->a_codec_ctx->channels;
     float size_per_sec = (n * sa_ctx->a_codec_ctx->sample_rate);
     
     AVPacket *packet = NULL, *pkt_temp = &(sa_ctx->pkt_temp);
     SAMutex_lock(sa_ctx->aq_lock); // to make sure SA_seek() frees ap *completely*.
     
NEXT_FRAME:

     while(packet == NULL)
     {
          SAMutex_lock(sa_ctx->apq_lock);
          packet = SAQ_pop(sa_ctx->apq_ctx);
          SAMutex_unlock(sa_ctx->apq_lock);

          if(packet == NULL)
               if(_SA_read_packet(sa_ctx) < 0)
               {
                    SAMutex_unlock(sa_ctx->aq_lock);
                    return NULL;
               }
     }

     float a_clock = -1.0;
     if(packet->pts != AV_NOPTS_VALUE)
          a_clock = av_q2d(sa_ctx->audio_st->time_base) * packet->pts;

     pkt_temp->data = packet->data;
     pkt_temp->size = packet->size;

     int data_size, decoded_size;
     while(pkt_temp->size > 0)
     {
          ret = malloc(sizeof(SAAudioPacket));
          if(ret != NULL)
               ret->abuffer = av_malloc(sizeof(uint8_t) * SAABUFFER_SIZE);
          if(ret == NULL || ret->abuffer == NULL)
          {
               if(ret != NULL)
               {
                    free(ret);
                    fprintf(stderr, "malloc failed on getting an abuffer\n");
               } else
                    fprintf(stderr, "failed on getting a SAAudioPacket buffer\n");

               av_free_packet(packet);
               av_free(packet);
               SAMutex_unlock(sa_ctx->aq_lock);
               return NULL;
          }

          data_size = sizeof(uint8_t) * SAABUFFER_SIZE;
          decoded_size = avcodec_decode_audio3(sa_ctx->a_codec_ctx,
                                               (int16_t *)(ret->abuffer),
                                               &data_size, pkt_temp);
          if(decoded_size <= 0)
          {
               av_free(ret->abuffer);
               free(ret);
               break; // skip this frame
          }

          pkt_temp->data += decoded_size;
          pkt_temp->size -= decoded_size;

          if(data_size <= 0)
          {
               av_free(ret->abuffer);
               free(ret);
               continue;
          }

          if(a_clock >= 0.0f)
               sa_ctx->a_clock = a_clock;

          ret->len = data_size;
          ret->pts = sa_ctx->a_clock;
          SAQ_push(sa_ctx->aq_ctx, ret);

          float delta_t = data_size / size_per_sec;
          sa_ctx->a_clock += delta_t;
          if(a_clock >= 0.0f)
               a_clock += delta_t;
     }

     av_free_packet(packet);
     av_free(packet);
     
     ret = SAQ_pop(sa_ctx->aq_ctx);
     if(ret == NULL)
     {
          packet = NULL;
          goto NEXT_FRAME;
     }

     SAMutex_unlock(sa_ctx->aq_lock);
     return ret;
}

void SA_free_ap(SAAudioPacket *ap)
{
     if(ap == NULL)
          return;
     av_free(ap->abuffer);
     free(ap);
}

void SA_free_vp(SAVideoPacket *vp)
{
     if(vp == NULL)
          return;
     free(vp);
}

int SA_seek(SAContext *sa_ctx, float seek_to, float delta)
{
     int ret = 0;
     SAMutex_lock(sa_ctx->aq_lock);
     SAMutex_lock(sa_ctx->apq_lock);
     SAMutex_lock(sa_ctx->vpq_lock);
     
     int64_t pos = seek_to * AV_TIME_BASE;
     int seek_flags = delta < 0 ? AVSEEK_FLAG_BACKWARD : 0;
     int stream_index = sa_ctx->v_stream;
     int64_t seek_target = av_rescale_q(pos, AV_TIME_BASE_Q,
                                        sa_ctx->video_st->time_base);
     if(av_seek_frame(sa_ctx->avfmt_ctx_ptr, stream_index, seek_target, seek_flags) < 0)
     {
          // FIXME: failed? EOF?
          // fprintf(stderr, "Seeking failed!\n");
          ret = -1;
     } else
     {
          void *ptr;

          while((ptr = SAQ_pop(sa_ctx->aq_ctx)) != NULL)
               SA_free_ap(ptr);
          
          while((ptr = SAQ_pop(sa_ctx->vpq_ctx)) != NULL)
               av_free_packet(ptr);
          
          while((ptr = SAQ_pop(sa_ctx->apq_ctx)) != NULL)
               av_free_packet(ptr);

          avcodec_flush_buffers(sa_ctx->a_codec_ctx);
          avcodec_flush_buffers(sa_ctx->v_codec_ctx);

          sa_ctx->video_clock = seek_to;
          sa_ctx->a_clock = seek_to;
     }

     SAMutex_unlock(sa_ctx->aq_lock);
     SAMutex_unlock(sa_ctx->vpq_lock);
     SAMutex_unlock(sa_ctx->apq_lock);
     
     return ret;
}

int _SA_get_buffer(struct AVCodecContext *c, AVFrame *pic)
{
     int ret = avcodec_default_get_buffer(c, pic);
     uint64_t *pts = av_malloc(sizeof(uint64_t));
     *pts = *(uint64_t *)(c->opaque);
     pic->opaque = pts;
     return ret;
}

void _SA_release_buffer(struct AVCodecContext *c, AVFrame *pic)
{
     if(pic)
          av_freep(&pic->opaque);
     avcodec_default_release_buffer(c, pic);
     return;
}

int SA_get_width(SAContext *sa_ctx)
{
     return sa_ctx->v_width;
}

int SA_get_height(SAContext *sa_ctx)
{
     return sa_ctx->v_height;
}

float SA_get_duration(SAContext *sa_ctx)
{
     return (float)(sa_ctx->avfmt_ctx_ptr->duration) / (float)(AV_TIME_BASE);
}

// FIXME: this will never works
//
// float SA_get_clock(void)
// {
//      int64_t t = av_gettime();
//      //return (double)(t / 1000000) + (double)((t % 1000000) / (double)1000000);
//      return t / 1000000.f;
// }

#ifdef __cplusplus
}
#endif
