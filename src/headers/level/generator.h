#ifndef GENERATOR_H
#define GENERATOR_H

#include "engine.h"
#include "game_area_controller.h"
#include <unordered_set>

#define MAPSIZE_SMALL 1

struct MapSettings {
    int map_size = 0;
    int style = 0;
};

static std::mt19937 RNG_generator;

static SDL_Color level_colors[4] = {
    { 0, 0, 0, 255},
    { 255, 0, 0, 255},
    { 0, 255, 0, 255},
    { 0, 0, 255, 255}

    // { 8, 0, 18, 255 } <- older color
};

inline Rectangle get_bounds(MapSettings &settings) {
    return { 0, 0, (int)gw * 2, (int)gh * 2 };
    /*
    // World bounds (depends on settings)
    if(settings.map_size == MAPSIZE_SMALL) {

    }*/
}

struct EnemySpawn {
    Vector2 position;
    int id;
};
static std::vector<EnemySpawn> generated_enemies(64);

void generate_enemies(int difficulty, int level, MapSettings &settings, Rectangle &world_bounds) {

    //if(level == 1) {
        generated_enemies.push_back({ Vector2(100, 100), 0 });
        generated_enemies.push_back({ Vector2(300, 100), 0 });
        generated_enemies.push_back({ Vector2(800, 100), 0 });
        generated_enemies.push_back({ Vector2(100, 200), 0 });
        generated_enemies.push_back({ Vector2(world_bounds.right() - 100, world_bounds.bottom() - 100), 0 });
    //}
}

// MAX level = 15
inline void generate(int seed, 
    int difficulty, 
    int level,
    MapSettings &settings, 
    GameAreaController *game_area_controller) {

    RNG_generator = std::mt19937(seed);

    Rectangle world_bounds = get_bounds(settings);
    game_area_controller->set_world_bounds(world_bounds);

    // Style - from settings
    SDL_Color c = level_colors[settings.style];
    renderer_set_clear_color(c);

    // Enemies (depends on settings? - no and yes? depends on difficulty and level also?)
    // same kind of enemy but different look depending on setting?
    generated_enemies.clear();
    generate_enemies(difficulty, level, settings, world_bounds);
    for(auto &e : generated_enemies) {
        // This is where we alter enemies with things from map?
        game_area_controller->spawn_target(e.position);
    }

    // Player start
    Vector2 player_position = world_bounds.center();
    Engine::logn("x: %d, y: %d, w: %d, h: %d", world_bounds.x, world_bounds.y, world_bounds.w, world_bounds.h);
    Engine::logn("x: %.2f, y: %.2f", player_position.x, player_position.y);
    camera_lookat(player_position);
    game_area_controller->spawn_player(player_position);

    // Boss - only activate on all other dead
        // conditional something or just a special type of variable somewhere?
    // (drops?)

}

struct IRDSObject {
    float rdsProbability; // The chance for this item to drop
    bool rdsUnique;        // Only drops once per query
    bool rdsAlways;        // Drops always
    int id;
};

struct RDSTable {
    std::vector<IRDSObject> rdsContents; // The contents of the table
};

inline std::vector<IRDSObject> rds(int rdsCount, RDSTable &table) {
    std::unordered_set<int> unique_drops;
    std::vector<IRDSObject> return_value;

    /*
    // Do the PreEvaluation on all objects contained in the current table
    // This is the moment where those objects might disable themselves.
    for (IRDSObject &o : table.rdsContents) {
        o.OnRDSPreResultEvaluation(EventArgs.Empty);
    }
    */

    // Add all the objects that are hit "Always" to the result
    // Those objects are really added always, no matter what "Count"
    // is set in the table! If there are 5 objects "always", those 5 will
    // drop, even if the count says only 3.
    for (IRDSObject &o : table.rdsContents) {
        if(!o.rdsAlways) {
            continue;
        }
        
        if(o.rdsUnique) {
            if(unique_drops.find(o.id) == unique_drops.end()) {
                return_value.push_back(o);
                unique_drops.insert(o.id);
            }
        } else {
            return_value.push_back(o);
        }
    }

    int alwayscnt = return_value.size();
    int realdropcnt = rdsCount - alwayscnt;

    if(realdropcnt <= 0) {
        return return_value;
    }

    for (int dropcount = 0; dropcount < realdropcnt; dropcount++) {
        float sum = 0.0f;

        // Calculate sum of all probabilities in the table that are not already added as always
        for(auto &o : table.rdsContents) {
            if(o.rdsAlways) {
                continue;
            }
            sum += o.rdsProbability;
        }
        
        // This is the magic random number that will decide, which object is hit now
        // public static double GetDoubleValue(double max)             // From 0.0 (incl) to max (excl)
        // double hitvalue = RDSRandom.GetDoubleValue(sum);
        float hitValue = RNG::range_f(0.0f, sum - 1.0f); // -1 to change to exclusive?
        
        // Find out in a loop which object's probability hits the random value...
        float runningvalue = 0;
        for(auto &o : table.rdsContents) {
            // Count up until we find the first item that exceeds the hitvalue...
            runningvalue += o.rdsProbability;
            if (hitValue < runningvalue) {
                // ...and the oscar goes too...
                if(o.rdsUnique) {
                    if(unique_drops.find(o.id) == unique_drops.end()) {
                        return_value.push_back(o);
                        unique_drops.insert(o.id);
                    }
                } else {
                    return_value.push_back(o);
                }
                break;
            }
        }
    }

    return return_value;
}

#endif