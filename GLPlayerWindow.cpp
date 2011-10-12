#include <iostream>

#ifndef __GLPLAYER__NO__DEBUG__
#include <cstdio>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "GLPlayerWindow.hpp"

GLPlayerWindow::GLPlayerWindow(QWidget *parent)
     : QGLWidget(parent), timer(NULL)
{
     setMouseTracking(true);
}

void GLPlayerWindow::startTimer() {
     if(timer != NULL)
          return;

     timer = new QTimer();
     connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
     timer->start(1000 / 60);
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
}

void GLPlayerWindow::initializeGL() {
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_COLOR_MATERIAL);
     glEnable(GL_TEXTURE_2D);
     glEnable(GL_BLEND);
     glEnable(GL_POLYGON_SMOOTH);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glClearColor(0, 0, 0, 0);
}

void GLPlayerWindow::resizeGL(int w, int h) {
     glViewport(0, 0, w, h);
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluOrtho2D(0, w, 0, h); // set origin to bottom left corner
     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();
}

void GLPlayerWindow::paintGL() {
     glClear(GL_COLOR_BUFFER_BIT);

     GLenum texture_format;
     GLint  nOfColors;

     SDL_Surface *frame = decoder.getFrame();
     
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
          std::cerr << "warning: the image is not truecolor..  this will probably break" << std::endl;
          // FIXME: this error should not go unhandled
     }
 
     glGenTextures( 1, &texture );
     glBindTexture( GL_TEXTURE_2D, texture );

#ifndef __GLPLAYER__NO__DEBUG__
     fprintf(stderr, "%d\n", (int)texture);
#endif

     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
     glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
 
     glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, frame->w, frame->h, 0,
                   texture_format, GL_UNSIGNED_BYTE, frame->pixels );

     glBegin(GL_QUADS);
          glTexCoord2f(0.0f, 0.0f); glVertex2f(0, this->height());
          glTexCoord2f(1.0f, 0.0f); glVertex2f(this->width(), this->height());
          glTexCoord2f(1.0f, 1.1f); glVertex2f(this->width(), 0);
          glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f);
     glEnd();
     glDeleteTextures(1, &texture);

     std::cout << "printGL() finished." << std::endl;
}

void GLPlayerWindow::keyPressEvent(QKeyEvent* event) {
     switch(event->key()) {
     case Qt::Key_Escape:
          close();
          break;
     default:
          event->ignore();
          break;
     }
}
