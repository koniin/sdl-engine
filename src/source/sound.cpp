#define USE_SDL_MIXER

#ifdef USE_SDL_MIXER

#include "engine.h"
#include "SDL_mixer.h"

static Mix_Chunk **sounds;
static size_t sound_count = 0;
static bool sound_disabled = false;

void sound_init() {
    SDL_Init(SDL_INIT_AUDIO);

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048 ) < 0 ) { 
        Engine::logn("SDL_mixer failed. Disabling sound. SDL_mixer Error: %s\n", Mix_GetError()); 
        sound_disabled = true;
        return;
    }

    sound_count = 0;
    sounds = new Mix_Chunk*[128];
}

size_t sound_load(std::string file) {
    if(sound_disabled) {
        return 0;
    }

    Mix_Chunk *sound = Mix_LoadWAV(file.c_str());
    ASSERT_WITH_MSG(sound != NULL, Text::format("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError()).c_str());
    
    sounds[sound_count] = sound;
    return sound_count++;
}

void sound_play(size_t id, int volume) {
    if(sound_disabled) {
        return;
    }
    /* Set the volume in the range of 0-128 of a specific channel or chunk.
        If the specified channel is -1, set volume for all channels.
        Returns the original volume.
        If the specified volume is -1, just return the current volume.
    */
    Mix_VolumeChunk(sounds[id], volume);
    // Channel is zero based
    Mix_PlayChannel(0, sounds[id], 0);
}

void sound_exit() {
    if(sound_disabled) {
        return;
    }

    for(size_t i = 0; i < sound_count; i++) {
        Mix_FreeChunk(sounds[i]);
    }
    
    Mix_Quit();
}

inline void sound_set_position() {
    /* Set the position of a channel. (angle) is an integer from 0 to 360, that
    *  specifies the location of the sound in relation to the listener. (angle)
    *  will be reduced as neccesary (540 becomes 180 degrees, -100 becomes 260).
    *  Angle 0 is due north, and rotates clockwise as the value increases.
    *  For efficiency, the precision of this effect may be limited (angles 1
    *  through 7 might all produce the same effect, 8 through 15 are equal, etc).
    *  (distance) is an integer between 0 and 255 that specifies the space
    *  between the sound and the listener. The larger the number, the further
    *  away the sound is. Using 255 does not guarantee that the channel will be
    *  culled from the mixing process or be completely silent. For efficiency,
    *  the precision of this effect may be limited (distance 0 through 5 might
    *  all produce the same effect, 6 through 10 are equal, etc). Setting (angle)
    *  and (distance) to 0 unregisters this effect, since the data would be
    *  unchanged.
    */
   
    // Mix_SetPosition(0, RNG::range_i(0, 360), RNG::range_i(0, 255));
}

#endif