/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#include <iostream>

#ifndef __GLPLAYER__NO__DEBUG__
#include <cstdio>
#endif

extern "C" {
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
}

#include "GLPlayerWindow.hpp"

GLPlayerWindow::GLPlayerWindow(QWidget *parent)
     : QGLWidget(parent), textureContainsData(false), timer(NULL),
       sasdlCtx(NULL), frame(NULL) {

     SDL_Init(SDL_INIT_AUDIO);
     SASDL_init();
     
     setMouseTracking(true);
}

GLPlayerWindow::~GLPlayerWindow() {
     timer->stop();
     
     if(textureContainsData) {
          glDeleteTextures(1, &texture);
     }
     Mix_CloseAudio();
     SASDL_close(sasdlCtx);
     SDL_FreeSurface(frame);
     SDL_Quit();
}

void GLPlayerWindow::startTimer() {
     if(timer != NULL)
          return;

     timer = new QTimer();
     connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));

     // FIXME: fixed FPS?
     // timer->start(1000 / 60);
     timer->start(0);
}

bool GLPlayerWindow::openVideoFile(std::string videoPath) {

     sasdlCtx = SASDL_open((char *)(videoPath.c_str()));
     if(sasdlCtx == NULL) {
          std::cerr << __FILE__ << ": Failed to open video file." << std::endl;
          return false;
     }

     if(Mix_OpenAudio(sasdlCtx->sa_ctx->a_codec_ctx->sample_rate, AUDIO_S16SYS,
                      sasdlCtx->sa_ctx->a_codec_ctx->channels, 512) < 0) {
          std::cerr << __FILE__ << ": Mix_OpenAudio: " << SDL_GetError() << std::endl;
          return false;
     }
     Mix_SetPostMix(SASDL_audio_decode, sasdlCtx);

     int width = SASDL_get_video_width(sasdlCtx);
     int height = SASDL_get_video_height(sasdlCtx);
     
     this->resize(width, height);
     frame = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                    0, 0, 0, 0);

     SASDL_play(sasdlCtx);
     return true;
}

void GLPlayerWindow::initializeGL() {
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_COLOR_MATERIAL);
     glEnable(GL_TEXTURE_2D);
     glEnable(GL_BLEND);
     glEnable(GL_POLYGON_SMOOTH);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glClearColor(0.0f, 0.0f, 0.0f, 0);
}

void GLPlayerWindow::resizeGL(int w, int h) {
     glViewport(0, 0, w, h);
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluOrtho2D(0, w, 0, h); // set origin to top left corner
     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();
}

void GLPlayerWindow::paintGL() {

     GLenum texture_format;
     GLint  nOfColors;

     if(SASDL_eof(sasdlCtx)) {
          // FIXME: or it is stopped?
          // FIXME: fill screen black?
          return;
     }

     // SDL_Surface *frame = decoder.getFrame();
     SASDL_draw(sasdlCtx, frame);

     nOfColors = frame->format->BytesPerPixel;
     if (nOfColors == 4) {
          if (frame->format->Rmask == 0x000000ff)
               texture_format = GL_RGBA;
          else
               texture_format = GL_BGRA;
     } else if (nOfColors == 3) {
          if (frame->format->Rmask == 0x000000ff)
               texture_format = GL_RGB;
          else
               texture_format = GL_BGR;
     } else {
          std::cerr << "warning: the image is not truecolor... "
                    << "this will probably break." << std::endl;
          // FIXME: this error should not go unhandled
     }

     if(textureContainsData) {
          glDeleteTextures(1, &texture);
     }
 
     glGenTextures(1, &texture);
     glBindTexture(GL_TEXTURE_2D, texture);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
     glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, frame->w, frame->h, 0,
                  texture_format, GL_UNSIGNED_BYTE, frame->pixels);

     textureContainsData = true;
     
     glBegin(GL_QUADS);
          glTexCoord2f(0.0f, 0.0f); glVertex2f(0, this->height());
          glTexCoord2f(1.0f, 0.0f); glVertex2f(this->width(), this->height());
          glTexCoord2f(1.0f, 1.0f); glVertex2f(this->width(), 0);
          glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
     glEnd();

#ifndef __GLPLAYER__NO__DEBUG__
#ifdef __GLPLAYER__DEBUG__CAL__FPS__

     // static int64_t prevFrameShowTime = 0;
     // std::cout << (double)1000000 / (double)(av_gettime() - prevFrameShowTime) << std::endl;
     // prevFrameShowTime = av_gettime();

     static int64_t prevFrameShowTime = 0;
     static int FPSCalCycleCount = -1;
     if(FPSCalCycleCount == -1) {
          FPSCalCycleCount = 0;
          prevFrameShowTime = av_gettime();
     } else if(FPSCalCycleCount == 5) {
          // This will also work.
          // std::cout << "Yoooooo FPS: " << (float)5000000 / (float)(av_gettime() - prevFrameShowTime)
          //           << std::endl;
          std::cout << "Current FPS: " << (double)5000000 / (double)(av_gettime() - prevFrameShowTime)
                    << std::endl;
          FPSCalCycleCount = 0;
          prevFrameShowTime = av_gettime();
     }
     FPSCalCycleCount++;

#endif
#endif

     // FIXME: will paintGL() be called while waiting for the next frame?
     SASDL_wait_for_next_frame(sasdlCtx);
}

void GLPlayerWindow::keyPressEvent(QKeyEvent* event) {

     // *FIXME*: add seeking/stopping support.
     
     switch(event->key()) {
     case Qt::Key_Escape:
          close();
          break;
     case Qt::Key_Space:
          switch(SASDL_get_video_status(sasdlCtx)) {
          case SASDL_is_playing:
          case SASDL_is_stopped:
               SASDL_play(sasdlCtx);
               break;
          case SASDL_is_paused:
               SASDL_pause(sasdlCtx);
               break;
          }
     default:
          event->ignore();
          break;
     }
}
