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

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "GLPlayerWindow.hpp"

GLPlayerWindow::GLPlayerWindow(QWidget *parent)
     : QGLWidget(parent), textureContainsData(false), timer(NULL) {
     
     setMouseTracking(true);
}

GLPlayerWindow::~GLPlayerWindow() {
     if(textureContainsData) {
          glDeleteTextures(1, &texture);
     }
}

void GLPlayerWindow::startTimer() {
     if(timer != NULL)
          return;

     timer = new QTimer();
     connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
     // timer->start(1000 / 60);
     timer->start(0);
}

void GLPlayerWindow::openVideoFile(std::string videoPath) {

#ifndef __GLPLAYER__NO__DEBUG__
     std::cerr << __FILE__ << ": opening the video..." << std::endl;
#endif
     
     decoder.openVideo(videoPath);
     
#ifndef __GLPLAYER__NO__DEBUG__
     std::cerr << __FILE__ << ": decoder successfully opened the video file." << std::endl;
#endif
     
     this->resize(decoder.getWidth(), decoder.getHeight());

#ifndef __GLPLAYER_NO__DEBUG__
     std::cerr << __FILE__ << ": window resized to the video size." << std::endl;
#endif

     decoder.start();
}

void GLPlayerWindow::initializeGL() {
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_COLOR_MATERIAL);
     glEnable(GL_TEXTURE_2D);
     glEnable(GL_BLEND);
     glEnable(GL_POLYGON_SMOOTH);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glClearColor(1.0f, 1.0f, 1.0f, 0);
}

void GLPlayerWindow::resizeGL(int w, int h) {
     glViewport(0, 0, w, h);
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluOrtho2D(0, w, 0, h); // set origin to top (bottom?) left corner
     // glOrtho(0, w, 0, h, 0, 0.000001); // set origin to top (bottom?) left corner
     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();
}

// FIXME: this will only work when computer is fast enough to
// keep video synced while not dropping any frame.
void GLPlayerWindow::paintGL() {

     GLenum texture_format;
     GLint  nOfColors;

     if(decoder.ends()) {
          // FIXME: fill black here

#ifndef __GLPLAYER__NO__DEBUG__
          std::cout << "Video ends!" << std::endl;
#endif
          return;
     }

     SDL_Surface *frame = decoder.getFrame();
     if(frame != NULL) {
          
#ifndef __GLPLAYER__NO__DEBUG__
          std::cout << "New frame acquired!" << std::endl;
#endif
          
#ifndef __GLPLAYER__NO__DEBUG__
#ifdef  __GLPLAYER__DEBUG__SAVE__FRAME__
          static int n = 0;
          char picPath[100];
          sprintf(picPath, "frame_%d.bmp", n++);
          SDL_SaveBMP(frame, picPath);
#endif
#endif

          nOfColors = frame->format->BytesPerPixel;
          if (nOfColors == 4) {     // contains an alpha channel
               if (frame->format->Rmask == 0x000000ff)
                    texture_format = GL_RGBA;
               else
                    texture_format = GL_BGRA;
          } else if (nOfColors == 3) {     // no alpha channel
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
     }

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
}

void GLPlayerWindow::keyPressEvent(QKeyEvent* event) {
     switch(event->key()) {
     case Qt::Key_Escape:
          close();
          break;
     case Qt::Key_Space:
          if(decoder.isPaused()) {
               decoder.start();
          } else if(decoder.isPlaying()) {
               decoder.pause();
          }
          break;
     default:
          event->ignore();
          break;
     }
}
