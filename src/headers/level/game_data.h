#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "engine.h"

enum MapSize {
    Small = 0,
    Medium = 1,
    Large = 2,
    SIZE_OF_MapSize = 3
};

enum MapStyle {
    Desert = 0,
    SIZE_OF_MapStyle = 1
};

static const char* MapSizeNames[] = { "Small", "Medium", "Large" };
static const char* MapStyleNames[] = { "Desert" };

// statically check that the sizes are correct
static_assert(sizeof(MapSizeNames)/sizeof(char*) == MapSize::SIZE_OF_MapSize, "MapSizeNames sizes dont match");
static_assert(sizeof(MapStyleNames)/sizeof(char*) == MapStyle::SIZE_OF_MapStyle, "MapStyleNames sizes dont match");

struct MapModifier {
    std::string name;
};

struct MapSettings {
    MapSize map_size;
    MapStyle style;
    std::vector<MapModifier> modifiers;
};

struct Upgrade {
    char *name;
    char *description;
};

static const size_t UPGRADE_COUNT = 3;
static const Upgrade Upgrades[UPGRADE_COUNT] = { 
    { "Blaster", "damage increase" },
    { "Help", "+1 MAX hp" }, 
    { "FASTER!", "Bullet speed increase" } 
};

enum Attack {
    Basic = 0,
    SIZE_OF_Attacks = 1
};

static const char* AttackNames[] = { "Basic" };
static_assert(sizeof(AttackNames)/sizeof(char*) == Attack::SIZE_OF_Attacks, "AttackNames sizes dont match");

struct Attack_t {
    float cooldown; // how much time between projectiles
    float accuracy; // how much the projectile can go of the straight line when fired (spreads in -angle to angle from initial angle)
    float projectile_speed; 
    float knockback;
    char *sound_name;
};

// Pixels per frame
constexpr float base_projectile() {
    return 8.0f / 0.016667f;
}

static const Attack_t Attacks[SIZE_OF_Attacks] = {
    { 0.25f, 8.0f, base_projectile(), 2.0f, "basic_fire"  }
};

// Pixels per frame
constexpr float player_move_acceleration() {
    return 10.0f / 0.016667f;
}

// degrees per frame
constexpr float player_move_rotation() {
    return 3.0f / 0.016667f;
}

// pixels per frame
constexpr float player_drag() {
    return 0.04f / 0.016667f;
}

struct PlayerState {
	float rotation_speed = player_move_rotation(); // degrees
	float move_acceleration = player_move_acceleration();
	float drag = player_drag();
    Attack attack = Attack::Basic;
};

struct GameState {
    int seed = 1338;
    int difficulty = 1;
    int level = 1;

    // player
    PlayerState player;
};

struct ProjectileData {
    int damage = 1;
    int radius = 0;

    ProjectileData(int damage, int radius) : damage(damage), radius(radius) {}
};

struct FireSettings {
    float fire_cooldown;
    float accuracy;
    float projectile_speed;
    float knockback;
    char *sound_name;
    ProjectileData p_data;

    FireSettings(float cooldown, float accuracy, float speed, float knockback, char *sound_name, ProjectileData data) :
        fire_cooldown(cooldown), accuracy(accuracy), projectile_speed(speed), knockback(knockback), sound_name(sound_name), p_data(data) {
    }
};

namespace GameData {
    void game_state_new();
    GameState *game_state_get();

    FireSettings create_fire_settings(const Attack &attack, const MapSettings &settings);
};

#endif