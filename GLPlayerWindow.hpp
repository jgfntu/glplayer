/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifndef __GLPLAYERWINDOW__HPP__DEFINED__
#define __GLPLAYERWINDOW__HPP__DEFINED__

#include <string>

#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>

#include "SimpleAV/SDL/SimpleAV_SDL.h"

class GLPlayerWindow : public QGLWidget
{
     Q_OBJECT

public:
     GLPlayerWindow(QWidget *parent = NULL);
     ~GLPlayerWindow();

     // it's ugly to seperate it from the constructor;
     // but we need to make sure OpenGL is fully initialized
     // before start drawing.
     //
     // PS: should we call this after loading video in the editor?
     void startTimer();
     
     void openVideoFile(std::string videoPath);

protected slots:
     void paintGL();
     
protected:

     void initializeGL();
     void resizeGL(int w, int h);
     // void mousePressEvent(QMouseEvent *event);
     // void mouseMoveEvent(QMouseEvent *event);
     void keyPressEvent(QKeyEvent *event);

     GLuint texture;
     bool textureContainsData;
     
     QTimer *timer;

     SASDLContext *sasdlCtx;
     SDL_Surface *frame;
};

#endif
