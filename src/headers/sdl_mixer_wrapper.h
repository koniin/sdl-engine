#ifndef SDL_MIXER_WRAPPER_H
#define SDL_MIXER_WRAPPER_H

#include "SDL_mixer.h"

static Mix_Chunk *gScratch = NULL;

void sdl_mix_init() {
    SDL_Init(SDL_INIT_AUDIO);

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) { 
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() ); 
    }
}

void sdl_mix_load(std::string file) {
    gScratch = Mix_LoadWAV(file.c_str()); 
    if( gScratch == NULL ) { 
        printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() ); 
    }
}

void sdl_mix_play() {
    Mix_PlayChannel( -1, gScratch, 0 );
}

void sdl_mix_exit() {
    Mix_FreeChunk( gScratch ); 
    gScratch = NULL;
    Mix_Quit();
}

#endif