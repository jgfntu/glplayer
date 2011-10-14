/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifndef __GLPLAYERDECODER__DEFINED__
#define __GLPLAYERDECODER__DEFINED__

#include <string>

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>
     
#include "SimpleAV/SimpleAV.h"
}

#define GLPlayerDecoder__Playing  0
#define GLPlayerDecoder__Stopped  1
#define GLPlayerDecoder__Paused   2

struct GLVideoPacket {
     SDL_Surface *surface;
     int64_t pts;
};

class GLPlayerDecoder {
     
public:
     GLPlayerDecoder();
     ~GLPlayerDecoder();

     void openVideo(std::string videoPath);

     // This function is compelety redundant.
     //
     // openVideo() will always
     // call closeVideo() before doing anything.
     //
     // So does the destructor.
     void closeVideo();
     
     SDL_Surface *getFrame();

     int getWidth();
     int getHeight();
     int getDuration();

     float getVideoClock();

     // *FIXME*: Be cautious of videoEOF!
     void start();
     void pause();
     // void stop();
     // void seek(float pos);

     bool isPlaying();
     bool isPaused();
     bool isStopped();

     bool ends();

protected:

     void convertAVFrameToSDLSurface(AVFrame *frame, SDL_Surface *surface);

     bool videoEOF, curFrameReturned;
     
     int64_t startTime;
     float restartAt;
     
     struct SwsContext *swsctx;
     SAContext *sactx;
     int width, height, duration;

     // be careful: NEVER free them, and you should
     // better not to modify them.
     SDL_Surface *curFrame, *nextFrame;
     
     float curFramePTS, nextFramePTS;

     int status;
};

#endif
