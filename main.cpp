#include <iostream>

#include <QApplication>

#include "GLPlayerWindow.hpp"

int main(int argc, char *argv[])
{
     QApplication app(argc, argv);

#ifndef __GLPLAYER__NO__DEBUG__
     std::cout << "number of arguments: " << argc << std::endl
               << argv[0] << std::endl
               << argv[1] << std::endl << std::endl;
#endif

     GLPlayerWindow window;
     window.resize(800, 600);
     window.show();

#ifndef __GLPLAYER__NO__DEBUG__
     std::cerr << __FILE__ << ": opening the video..." << std::endl;
#endif
     
     window.openVideoFile(argv[1]);

#ifndef __GLPLAYER__NO__DEBUG__
     std::cerr << __FILE__ << ": timer will be started!" << std::endl;
#endif
     
     window.startTimer();

     return app.exec();
}
