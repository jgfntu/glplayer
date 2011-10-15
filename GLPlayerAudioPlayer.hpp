#ifndef __GLPLAYERAUDIOPLAYER__HPP__DEFINED__
#define __GLPLAYERAUDIOPLAYER__HPP__DEFINED__

#include "GLPlayerDecoder.hpp"

#define NUM_BUFFERS  3
#define BUFFER_SIZE  32256

// *FIXME*: define BUFFER_SIZE here?

class GLPlayerAudioPlayer {
     
public:
     GLPlayerAudioPlayer(GLPlayerDecoder *d);
     ~GLPlayerAudioPlayer();

     void run();

private:

     bool isRunning;
     GLPlayerDecoder *decoder;

     // Copied from kcat's example
     
     ALCdevice *dev;
     ALCcontext *ctx;
     struct stat statbuf;

     ALuint source, buffers[NUM_BUFFERS];
     ALuint frequency;
     ALenum format;

}

#endif
