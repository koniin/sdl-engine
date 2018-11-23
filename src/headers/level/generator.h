#ifndef GENERATOR_H
#define GENERATOR_H

#include "engine.h"
#include "game_area_controller.h"
#include <unordered_set>

#define MAPSIZE_SMALL 1

#define MAPSTYLE_DESERT 0

struct MapModifier {

};

struct MapSettings {
    int map_size = 0;
    int style = 0;
    std::vector<MapModifier> modifiers;
};

namespace GENRNG {
	static std::mt19937 RNG_generator;

    static void init(int seed) {
        RNG_generator = std::mt19937(seed);
    }

    inline int next_i(int max) {
        std::uniform_int_distribution<int> range(0, max);
        return range(RNG_generator);
    }

    inline int range_i(int min, int max) {
        std::uniform_int_distribution<int> range(min, max);
        return range(RNG_generator);
    }

    inline float range_f(float min, float max) {
        std::uniform_real_distribution<float> range(min, max);
        return range(RNG_generator);
    }

	inline void random_point_i(int xMax, int yMax, int &xOut, int &yOut) {
		std::uniform_int_distribution<int> xgen(0, xMax);
		std::uniform_int_distribution<int> ygen(0, yMax);
		xOut = xgen(RNG_generator);
		yOut = ygen(RNG_generator);
	}

	inline Vector2 vector2(const float &x_min, const float &x_max, const float &y_min, const float &y_max) {
		std::uniform_real_distribution<float> xgen(x_min, x_max);
		std::uniform_real_distribution<float> ygen(y_min, y_max);
		return Vector2(xgen(RNG_generator), ygen(RNG_generator));
	}
}

// To generate different options for the player
void generate_settings(int seed, int difficulty, int level, MapSettings &settings) {
    // https://pathofexile.gamepedia.com/List_of_map_mods
    settings.map_size = MAPSIZE_SMALL;
    settings.style = MAPSTYLE_DESERT;

    // settings.modifiers = { MapModifier() };
}

static SDL_Color level_colors[4] = {
    { 166, 130, 86, 255 }, // Sand
    { 255, 0, 0, 255},
    { 0, 255, 0, 255},
    { 0, 0, 255, 255}
};

struct EnemySpawn {
    Vector2 position;
    int id;
};
static std::vector<EnemySpawn> generated_enemies(64);

void generate_enemies(int difficulty, int level, const MapSettings &settings, Rectangle &world_bounds) {
    for(int i = 0; i < 5; i++) {
        generated_enemies.push_back(
            { 
                GENRNG::vector2(10.0f, (float)world_bounds.right() - 10.0f, 10.0f, (float)world_bounds.bottom() - 10.0f)
                , 0 
            }
        );
    }
}

inline Rectangle get_bounds(const MapSettings &settings) {
    return { 0, 0, (int)gw * 2, (int)gh * 2 };
    /*
    // World bounds (depends on settings)
    if(settings.map_size == MAPSIZE_SMALL) {

    }*/
}

// MAX level = 15
inline void generate_level(int seed, 
    int difficulty, 
    int level,
    const MapSettings &settings, 
    GameAreaController *game_area_controller) {

    GENRNG::init(seed);
    
    Rectangle world_bounds = get_bounds(settings);
    game_area_controller->set_world_bounds(world_bounds);

    // Style - from settings
    SDL_Color c = level_colors[settings.style];
    game_area_controller->set_background_color(c);

    // Enemies (depends on settings? - no and yes? depends on difficulty and level also?)
    // same kind of enemy but different look depending on setting?
    generated_enemies.clear();
    generate_enemies(difficulty, level, settings, world_bounds);
    for(auto &e : generated_enemies) {
        // This is where we alter enemies with things from map?
        // settings.modifiers
        game_area_controller->spawn_target(e.position);
    }

    // Player start
    Vector2 player_position = world_bounds.center();
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