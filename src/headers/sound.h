#ifndef SOUND_H
#define SOUND_H

#include <string>

void sound_init();
size_t sound_load(std::string file);
void sound_play(size_t id, int volume);
void sound_exit();

#endif