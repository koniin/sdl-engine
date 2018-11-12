#ifndef SDL_MIXER_WRAPPER_H
#define SDL_MIXER_WRAPPER_H

#include "SDL_mixer.h"

static Mix_Chunk *gScratch = NULL;

static Mix_Chunk **sounds;
static size_t sound_count = 0;

inline void sdl_mix_init() {
    SDL_Init(SDL_INIT_AUDIO);

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048 ) < 0 ) { 
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

// extern DECLSPEC int SDLCALL Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);

inline void sdl_mix_play(size_t id, int volume) {
    Mix_SetPosition(0, RNG::range_i(0, 360), RNG::range_i(0, 255));
    /* Set the volume in the range of 0-128 of a specific channel or chunk.
        If the specified channel is -1, set volume for all channels.
        Returns the original volume.
        If the specified volume is -1, just return the current volume.
    */
    Mix_VolumeChunk(sounds[id], volume);
    // Channel is zero based
    Mix_PlayChannel(0, sounds[id], 0);
}

inline void sdl_mix_exit() {
    for(size_t i = 0; i < sound_count; i++) {
        Mix_FreeChunk(sounds[i]);
    }
    
    Mix_Quit();
}

#endif