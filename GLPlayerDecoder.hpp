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
     
     // float getTime();
     SDL_Surface *getFrame();

     int getWidth();
     int getHeight();
     int getDuration();

     // void start();
     // void pause();
     // void stop();
     // void seek(float pos);

protected:

     void convertAVFrameToSDLSurface(AVFrame *frame, SDL_Surface *surface);
     
     // int startTime, restartAt;
     struct SwsContext *swsctx;
     SAContext *sactx;
     int width, height, duration;

     // be careful: NEVER free them, and you should
     // better not to modify them.
     SDL_Surface *curFrame, *nextFrame;
     
     float curFramePTS, nextFramePTS;
};

#endif
