#ifndef __GLPLAYERWINDOW__HPP__DEFINED__
#define __GLPLAYERWINDOW__HPP__DEFINED__

#include <string>

#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>

#include "GLPlayerDecoder.hpp"

class GLPlayerWindow : public QGLWidget
{
     Q_OBJECT

public:
     GLPlayerWindow(QWidget *parent = NULL);

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
     
     QTimer *timer;
     GLPlayerDecoder decoder;
};

#endif
