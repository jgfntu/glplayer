#include <iostream>

#include "GLPlayerDecoder.hpp"

// FIXME: integrate a Saya-specified version of SimpleAV later.

GLPlayerDecoder::GLPlayerDecoder()
     : swsctx(NULL), sactx(NULL), curFrame(NULL), nextFrame(NULL),
       curFramePTS(0.0f), nextFramePTS(0.0f) {
     
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

// FIXME: to be finished
/*
void GLPlayerDecoder::start() {
     
}
*/

// FIXME: to be finished
SDL_Surface *GLPlayerDecoder::getFrame() {
     // *FIXME*: this is only for "fast-forwarding".

     // This is necessary because we always use the same memory for these 2 frames.
     // what we do is in fact "curFrame <- nextFrame".
     SDL_Surface *t = curFrame;
     curFrame = nextFrame;
     nextFrame = t;

     curFramePTS = nextFramePTS;
     

     SAVideoPacket *vp = SA_get_vp(sactx);
     nextFramePTS = vp->pts;
     convertAVFrameToSDLSurface(vp->frame_ptr, nextFrame);
     SA_free_vp(vp);
     
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
