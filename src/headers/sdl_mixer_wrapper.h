#ifndef SDL_MIXER_WRAPPER_H
#define SDL_MIXER_WRAPPER_H

#include "SDL_mixer.h"

static Mix_Chunk *gScratch = NULL;

static Mix_Chunk **sounds;
static size_t sound_count = 0;

inline void sdl_mix_init() {
    SDL_Init(SDL_INIT_AUDIO);

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) { 
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() ); 
    }

    sound_count = 0;
    sounds = new Mix_Chunk*[128];
}

inline size_t sdl_mix_load(std::string file) {
    Mix_Chunk *sound = Mix_LoadWAV(file.c_str());
    ASSERT_WITH_MSG(sound != NULL, Text::format("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError()).c_str());
    
    sounds[sound_count] = sound;
    return sound_count++;
}

inline void sdl_mix_play(size_t id) {
    Mix_PlayChannel( -1, sounds[id], 0 );
}

inline void sdl_mix_exit() {
    for(size_t i = 0; i < sound_count; i++) {
        Mix_FreeChunk(sounds[i]);
    }
    
    Mix_Quit();
}

#endif