#ifndef GENERATOR_H
#define GENERATOR_H

#include "game_area_controller.h"

struct MapSettings {};

inline void generate(int seed, 
    int difficulty, 
    MapSettings settings, 
    GameAreaController *game_area_controller) {

    // World bounds (depends on settings)
    // Style (depends on settings)
    // Enemies (depends on settings)
    // Player start
    // Boss - only activate on all other dead
        // conditional something or just a special type of variable somewhere?
    // (drops?)

}

#endif