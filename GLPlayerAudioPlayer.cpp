#include <iostream>

#include <AL/al.h>
#include <AL/alc.h>

#include "GLPlayerAudioPlayer.hpp"

// *FIXME*: not finished

GLPlayerAudioPlayer::GLPlayerAudioPlayer(GLPlayerDecoder *d)
     : isRunning(false), decoder(d) {

     // initialize OpenAL here.
     dev = alcOpenDevice(NULL);
     if(dev == NULL) {
          std::cerr << __FILE__ << ": Failed to open device." << std::endl;
          // FIXME: handle this!
     }

     ctx = alcCreateContext(dev, NULL);
     alcMakeContextCurrent(ctx);
     if(ctx == NULL) {
          std::cerr << __FILE__ << ": Failed to create context." << std::endl;
     }

     alGenBuffers(NUM_BUFFERS, buffers);
     alGenSources(1, &source);

     // from here: mpstream.c, line 183 / alffmpeg.c

     // int bits;
     ALuint rate;
     ALenum channels, type, format = 0;
     AVCodecContext *aCodecCtx = (decoder->sactx).a_codec_ctx;

     rate = aCodecCtx->sample_rate;
     // frequency = aCodecCtx->sample_rate;
     
     switch(aCodecCtx->channel_layout) {
     case AV_CH_LAYOUT_MONO:       channels = AL_MONO;       break;
     case AV_CH_LAYOUT_STEREO:     channels = AL_STEREO;     break;
     case AV_CH_LAYOUT_QUAD:       channels = AL_QUAD;       break;
     case AV_CH_LAYOUT_5POINT1:    channels = AL_5POINT1;    break;
     case AV_CH_LAYOUT_7POINT1:    channels = AL_7POINT1;    break;
     default:
          std::cerr << __FILE__ << ": unreconized channel type." << std::endl;
     }

     switch(aCodecCtx->sample_fmt) {
     case AV_SAMPLE_FMT_U8:     type = AL_UNSIGNED_BYTE;     break;
     case AV_SAMPLE_FMT_S16:    type = AL_SHORT;             break;
     case AV_SAMPLE_FMT_S32:    type = AL_INT;               break;
     case AV_SAMPLE_FMT_FLT:    type = AL_FLOAT;             break;
     case AV_SAMPLE_FMT_DBL:    type = AL_DOUBLE;            break;
     default:
          std::cerr << __FILE__ << ": unreconized sample type." << std::endl;
     }

     if(type == AL_UNSIGNED_BYTE)
     {
          if(channels == AL_MONO) format = AL_FORMAT_MONO8;
          else if(channels == AL_STEREO) format = AL_FORMAT_STEREO8;
          else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
          {
               if(channels == AL_QUAD) format = alGetEnumValue("AL_FORMAT_QUAD8");
               else if(channels == AL_5POINT1) format = alGetEnumValue("AL_FORMAT_51CHN8");
               else if(channels == AL_7POINT1) format = alGetEnumValue("AL_FORMAT_71CHN8");
          }
     }
     else if(type == AL_SHORT)
     {
          if(channels == AL_MONO) format = AL_FORMAT_MONO16;
          else if(channels == AL_STEREO) format = AL_FORMAT_STEREO16;
          else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
          {
               if(channels == AL_QUAD) format = alGetEnumValue("AL_FORMAT_QUAD16");
               else if(channels == AL_5POINT1) format = alGetEnumValue("AL_FORMAT_51CHN16");
               else if(channels == AL_7POINT1) format = alGetEnumValue("AL_FORMAT_71CHN16");
          }
     }
     else if(type == AL_FLOAT && alIsExtensionPresent("AL_EXT_FLOAT32"))
     {
          if(channels == AL_MONO) format = alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
          else if(channels == AL_STEREO) format = alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
          else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
          {
               if(channels == AL_QUAD) format = alGetEnumValue("AL_FORMAT_QUAD32");
               else if(channels == AL_5POINT1) format = alGetEnumValue("AL_FORMAT_51CHN32");
               else if(channels == AL_7POINT1) format = alGetEnumValue("AL_FORMAT_71CHN32");
          }
     }
     else if(type == AL_DOUBLE && alIsExtensionPresent("AL_EXT_DOUBLE"))
     {
          if(channels == AL_MONO) format = alGetEnumValue("AL_FORMAT_MONO_DOUBLE");
          else if(channels == AL_STEREO) format = alGetEnumValue("AL_FORMAT_STEREO_DOUBLE");
     }

     if(format == 0 || format == -1) {
          std::cerr << __FILE__ << ": Unhandled format." << std::endl;
     }
}

GLPlayerAudioPlayer::~GLPlayerAudioPlayer() {
     if(isRunning()) {
          // FIXME: stop running
     }

     // FIXME: close OpenAL, etc...
}

void GLPlayerAudioPlayer::run() {
     // FIXME: run this in another thread
}
