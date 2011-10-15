/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <stdio.h>

#include "SimpleAV_SDL.h"

int main(int argc, char *argv[])
{
     if(argc != 2)
     {
          fprintf(stderr, "Usage:\nsaplayer <filename>\n");
          return 1;
     }
     
     // FIXME: add SDL_INIT_EVENTTHREAD on Windows and Mac?
     SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
     
     SASDL_init();
     SASDLContext *sasdl_ctx = SASDL_open(argv[1]);
     if(sasdl_ctx == NULL)
     {
          fprintf(stderr, "failed to open video file?\n");
          SDL_Quit();
          return 1;
     }

     // still very ugly...
     if(Mix_OpenAudio(sasdl_ctx->sa_ctx->a_codec_ctx->sample_rate, AUDIO_S16SYS,
                      sasdl_ctx->sa_ctx->a_codec_ctx->channels, 512) < 0) {
          fprintf(stderr, "Mix_OpenAudio: %s\n", SDL_GetError());
          SASDL_close(sasdl_ctx);
          SDL_Quit();
          return 1;
     }
     Mix_SetPostMix(SASDL_audio_decode, sasdl_ctx);

     double delta = 0.0f;
     SDL_Event event;
     int width = SASDL_get_video_width(sasdl_ctx);
     int height = SASDL_get_video_height(sasdl_ctx);
     
     SDL_Surface *screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
     int (*get_event)(SDL_Event *) = SDL_PollEvent;
     
     printf("video duration: %.3fs\n", SASDL_get_video_duration(sasdl_ctx));
     SASDL_play(sasdl_ctx);

     while(SASDL_eof(sasdl_ctx) == FALSE)
     {
          SASDL_draw(sasdl_ctx, screen);
          SDL_Flip(screen);

          while(get_event(&event))
               if(event.type == SDL_QUIT) {
                    Mix_CloseAudio();
                    SASDL_close(sasdl_ctx);
                    goto PROGRAM_QUIT;
               } else if(event.type == SDL_KEYDOWN) {
                    switch(event.key.keysym.sym) {
                    case SDLK_LEFT:
                         delta = -10.0;
                         break;
                    case SDLK_RIGHT:
                         delta = 10.0;
                         break;
                    case SDLK_UP:
                         delta = -60.0;
                         break;
                    case SDLK_DOWN:
                         delta = 60.0;
                         break;
                    case SDLK_SPACE:
                         if(SASDL_get_video_status(sasdl_ctx) == SASDL_is_playing)
                         {
                              SASDL_pause(sasdl_ctx);
                              get_event = SDL_WaitEvent;
                              continue;
                         } else
                         {
                              SASDL_play(sasdl_ctx);
                              get_event = SDL_PollEvent;
                              goto NEXT_LOOP;
                         }
                    case SDLK_s:
                         SASDL_stop(sasdl_ctx);
                         SASDL_draw(sasdl_ctx, screen); // fill screen with black
                         SDL_Flip(screen);
                         get_event = SDL_WaitEvent;
                         continue;
                    default:
                         // ignore this event. get the next one.
                         continue;
                    }

                    if(SASDL_seek(sasdl_ctx, SASDL_get_video_clock(sasdl_ctx) + delta) < 0)
                    {
                         Mix_CloseAudio();
                         SASDL_close(sasdl_ctx);
                         goto PROGRAM_QUIT;
                    }

                    SASDL_draw(sasdl_ctx, screen);
                    SDL_Flip(screen);
               }

          SASDL_wait_for_next_frame(sasdl_ctx);
     NEXT_LOOP:;
     }
                    
PROGRAM_QUIT:
     SDL_Quit();
     return 0;
}

#ifdef __cplusplus
}
#endif
