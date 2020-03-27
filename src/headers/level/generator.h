#ifndef GENERATOR_H
#define GENERATOR_H

#include "engine.h"
#include "game_area_controller.h"
#include "game_data.h"

#include <unordered_set>
#include <algorithm>

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
void generate_settings(MapSettings &settings) {
    // GameState *game_state = GameData::game_state_get();
    // game_state->difficulty
    // game_state->level

    // Difficulty could also be that you get harder modifiers
    // difficulty modifier that increases every map modifier

    // we could do a random range something like this: 
    // int range = difficulty * level;
    // max(range_i(range / 2, range), 10)
    size_t modifiers_to_generate = 2;

    settings.map_size = MapSize::Small;
    settings.style = MapStyle::Desert;

    auto &modifiers = GameData::get_map_modifiers();
    std::vector<int> _gen_choices(modifiers.size());
    int max = modifiers.size();
    for(int i = 0; i < max; i++) {
        _gen_choices[i] = i;
    }
    std::shuffle(std::begin(_gen_choices), std::end(_gen_choices), GENRNG::RNG_generator);

    for(size_t i = 0; i < modifiers_to_generate; i++) {
        settings.modifiers.push_back(modifiers[_gen_choices[i]]);
    }
}

void generate_random_upgrades(std::vector<Upgrade> &upgrade_choices, const size_t &count) {
    std::vector<int> _gen_choices;
    _gen_choices.reserve(count);
    
    auto &upgrades = GameData::get_upgrades();
    int max = upgrades.size();

    for(int i = 0; i < max; i++) {
        _gen_choices.push_back(i);
    }
    std::shuffle(std::begin(_gen_choices), std::end(_gen_choices), GENRNG::RNG_generator);
    for(size_t i = 0; i < count; i++) {
        upgrade_choices.push_back(upgrades[_gen_choices[i]]);
    }
}

static SDL_Color level_colors[4] = {
    { 166, 130, 86, 255 }, // Sand
    { 255, 0, 0, 255},
    { 0, 255, 0, 255},
    { 0, 0, 255, 255}
};

struct EnemySpawn {
    Vector2 position;
    Enemy e;
};
static std::vector<EnemySpawn> generated_enemies(64);

static const std::vector<Enemy> enemy_types = { 
    { 5, 5, 8, 100.0f, 100.0f, 200.0f, 10.0f }, // => activates at short range, follows player, if too close it moves in random dir
    { 5, 5, 8, 100.0f, 300.0f, 600000.0f, 10.0f } // => activates at "normal" range, always moves in random dir
};

static const std::vector<Enemy> boss_types = { 
    { 10, 10, 8, 100.0f, 300.0f, 200.0f, 10.0f }, // => activates at short range, follows player, if too close it moves in random dir
};

void generate_group() {
    /*

struct GenRoom {
	SDL_Rect rect;
};

static const int r_width = 20;
static const int r_height = 15;
static std::vector<GenRoom> rooms;
static const std::vector<Engine::Point> directions = {
	{ r_width, 0},
	{ 0, r_height},
	{ -r_width, 0},
	{ 0, -r_height}
};

static void add_room() {
	GenRoom nr = {{gw / 2, gh / 2, r_width, r_height}};

	bool generate = true;
	while(generate) {
		generate = false;
		for(auto &r : rooms) {
			if(r.rect.x == nr.rect.x && r.rect.y == nr.rect.y) {
				auto& dir = directions[RNG::range(0, directions.size())];
				nr.rect.x += dir.x;
				nr.rect.y += dir.y;
				// save r.rect as last neighbour to generate a connection between them
				generate = true;
				break;
			}
		}
	}	
	// make connection here to last rect
	rooms.push_back(nr);
}

static void generate_rooms() {
	GenRoom nr = {{gw / 2, gh / 2, r_width, r_height}};
	rooms.push_back(nr);

	for(int i = 0; i < 20; i++) {
		add_room();
	}
}
    */
}

void generate_enemies(const MapSettings &settings, Rectangle &world_bounds) {
    // GameState *game_state = GameData::game_state_get();
    // game_state->difficulty
    // game_state->level
    int enemies_to_generate = 1;
    for(int i = 0; i < enemies_to_generate; i++) {
        Enemy enemy = enemy_types[0];
        
        settings.apply_enemy_modifiers(enemy);

        generated_enemies.push_back(
            { 
                GENRNG::vector2(10.0f, (float)world_bounds.right() - 10.0f, 10.0f, (float)world_bounds.bottom() - 10.0f), 
                enemy
            }
        );
    }
}

Enemy generate_boss(const MapSettings &settings) {
    Enemy enemy = boss_types[0];
    settings.apply_enemy_modifiers(enemy);
    return enemy;
}

inline Rectangle get_bounds(const MapSettings &settings) {
    return { 0, 0, (int)gw * 2, (int)gh * 2 };
    /*
    // World bounds (depends on settings)
    if(settings.map_size == MAPSIZE_SMALL) {

    }*/
}

// MAX level = 15
inline void generate_level(
    const MapSettings &settings, 
    GameAreaController *game_area_controller) {

    GameState *game_state = GameData::game_state_get();

    GENRNG::init(game_state->seed);
    
    game_area_controller->set_settings(settings);

    Rectangle world_bounds = get_bounds(settings);
    game_area_controller->set_world_bounds(world_bounds);

    // Style - from settings
    SDL_Color c = level_colors[settings.style];
    game_area_controller->set_background_color(c);

    // Enemies (depends on settings? - no and yes? depends on difficulty and level also?)
    // same kind of enemy but different look depending on setting?
    generated_enemies.clear();
    generate_enemies(settings, world_bounds);
    for(auto &e : generated_enemies) {
        game_area_controller->spawn_target(e.position, e.e);
    }

    auto boss = generate_boss(settings);
    game_area_controller->set_boss(boss);

    // Player start
    Vector2 player_position = world_bounds.center();
    camera_lookat(player_position);
    game_area_controller->spawn_player(player_position);
}

#endif