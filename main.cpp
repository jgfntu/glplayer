/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#include <iostream>

#include <QApplication>

#include "GLPlayerWindow.hpp"

int main(int argc, char *argv[])
{
     if(argc != 2) {
          std::cout << "Glplayer Usage:" << std::endl
                    << "glplayer <filename>" << std::endl;
          return 1;
     }
     
     QApplication app(argc, argv);

     GLPlayerWindow window;
     window.resize(800, 600);
     window.show();

     if(window.openVideoFile(argv[1]) == false) {
          std::cout << "Failed to open file " << argv[1] << std::endl;
          return 1;
     }
     
     window.startTimer();

     return app.exec();
}
