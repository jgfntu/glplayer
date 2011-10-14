/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#include <iostream>

#include "GLPlayerDecoder.hpp"

// FIXME: integrate a Saya-specified version of SimpleAV later.

GLPlayerDecoder::GLPlayerDecoder()
     : videoEOF(false), curFrameReturned(false),
       swsctx(NULL), sactx(NULL), curFrame(NULL), nextFrame(NULL),
       curFramePTS(0.0f), nextFramePTS(0.0f), status(GLPlayerDecoder__Stopped) {
     
     SA_init();
}

GLPlayerDecoder::~GLPlayerDecoder() {
     closeVideo();
}

void GLPlayerDecoder::openVideo(std::string videoPath) {
     closeVideo();
     
     sactx = SA_open((char *)(videoPath.c_str()));
     if(sactx == NULL) {
          std::cerr << "SA_open failed." << std::endl;
          // FIXME: what to do?
     }

     width = SA_get_width(sactx);
     height = SA_get_height(sactx);
     duration = SA_get_duration(sactx);

     curFrame = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
     nextFrame = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);

     swsctx = sws_getContext(width, height, sactx->v_codec_ctx->pix_fmt,
                             width, height, PIX_FMT_RGB32, SWS_FAST_BILINEAR,
                             NULL, NULL, NULL);
     if(swsctx == NULL) {
          std::cerr << "Failed to get scaling context." << std::endl;
          // FIXME: what to do?
     }

     SAVideoPacket *vp = SA_get_vp(sactx);
     curFramePTS = vp->pts;
     convertAVFrameToSDLSurface(vp->frame_ptr, curFrame);
     SA_free_vp(vp);

     vp = SA_get_vp(sactx);
     nextFramePTS = vp->pts;
     convertAVFrameToSDLSurface(vp->frame_ptr, nextFrame);
     SA_free_vp(vp);
}

void GLPlayerDecoder::closeVideo() {
     if(sactx != NULL)
          SA_close(sactx);
     if(swsctx != NULL)
          sws_freeContext(swsctx);
     if(curFrame != NULL)
          SDL_FreeSurface(curFrame);
     if(nextFrame != NULL)
          SDL_FreeSurface(nextFrame);
     sactx = NULL;
     swsctx = NULL;
     curFrame = nextFrame = NULL;
}

// FIXME: should be revised to make it work with stop() and seek().
void GLPlayerDecoder::start() {
     if(isPaused()) {
          startTime = av_gettime() - (int64_t)(restartAt * 1000000.0f);
     } else {
          startTime = av_gettime();
     }
     
     status = GLPlayerDecoder__Playing;
}

// FIXME: should be revised to make it work with stop() and seek().
void GLPlayerDecoder::pause() {
     if(isPaused()) {
          return;
     } else {
          restartAt = getVideoClock();
          status = GLPlayerDecoder__Paused;
     }
}

SDL_Surface *GLPlayerDecoder::getFrame() {

     if(videoEOF || isPaused()) {
          return NULL;
     }

     float videoClock = getVideoClock();

     if(curFrameReturned == false &&
        curFramePTS != 1.0f && nextFramePTS != 1.0f &&
        curFramePTS <= videoClock && videoClock < nextFramePTS) {
          curFrameReturned = true;
          return NULL;
     }
     
     if(nextFramePTS != -1.0f && nextFramePTS <= videoClock) {
          
          // This is necessary because we always use the same memory for these 2 frames.
          // what we do is in fact "curFrame <- nextFrame".
          SDL_Surface *t = curFrame;
          curFrame = nextFrame;
          nextFrame = t;
          curFramePTS = nextFramePTS;
          

          SAVideoPacket *vp = SA_get_vp(sactx);
          if(vp == NULL) {
               nextFramePTS = -1.0f;
          } else {
               nextFramePTS = vp->pts;
               convertAVFrameToSDLSurface(vp->frame_ptr, nextFrame);
               SA_free_vp(vp);
          }
     }

     if(curFramePTS <= videoClock && nextFramePTS == -1.0f) {
          videoEOF = true;
          curFramePTS = -1.0f;
     }
     return curFrame;
}

int GLPlayerDecoder::getWidth() {
     return width;
}

int GLPlayerDecoder::getHeight() {
     return height;
}

int GLPlayerDecoder::getDuration() {
     return duration;
}

float GLPlayerDecoder::getVideoClock() {
     if(isPlaying()) {
          return (float)(av_gettime() - startTime) / 1000000.0f;
     } else if(isPaused()) {
          return restartAt;
     } else {
          return 0.0f;
     }
}

bool GLPlayerDecoder::isPlaying() {
     return (status == GLPlayerDecoder__Playing);
}

bool GLPlayerDecoder::isPaused() {
     return (status == GLPlayerDecoder__Paused);
}

bool GLPlayerDecoder::isStopped() {
     return (status == GLPlayerDecoder__Stopped);
}

bool GLPlayerDecoder::ends() {
     return videoEOF;
}

// helper functions

void GLPlayerDecoder::convertAVFrameToSDLSurface(AVFrame *frame,
                                                 SDL_Surface *surface) {
     AVPicture pict;
     
     SDL_LockSurface(surface);

     pict.data[0] = (uint8_t *)(surface->pixels);
     pict.linesize[0] = surface->pitch;
     sws_scale(swsctx, (const uint8_t * const *)(frame->data),
               frame->linesize, 0, height, pict.data, pict.linesize);
     
     SDL_UnlockSurface(surface);
}
