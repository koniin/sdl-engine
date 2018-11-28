#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "engine.h"

// Pixels per frame
constexpr float base_projectile_speed() {
    return 8.0f / 0.016667f;
}
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

// Should be like 
// name 
// description
// modifier (for drops)

struct MapModifier {
    char *name;
    char *description;
    float drop_modifier;

    // -> actual modifier (function?)
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

/// --------------
/// Attacks

enum Attack {
    Basic = 0,
    SIZE_OF_Attacks = 1
};

static const char* AttackNames[] = { "Basic" };
static_assert(sizeof(AttackNames)/sizeof(char*) == Attack::SIZE_OF_Attacks, "AttackNames sizes dont match");

struct Attack_t {
    char *sound_name;
    float cooldown; // how much time between projectiles
    float accuracy; // how much the projectile can go of the straight line when fired (spreads in -angle to angle from initial angle)
    
    float knockback;
    float range;
    float projectile_speed; 
    int projectile_damage;
    int projectile_radius;
};

static const Attack_t Attacks[SIZE_OF_Attacks] = {
    { "basic_fire", 0.25f, 8.0f, 2.0f, 0.8f, base_projectile_speed(), 1, 8 }
};

/// --------------

struct PlayerState {
	float rotation_speed = player_move_rotation(); // degrees
	float move_acceleration = player_move_acceleration();
	float drag = player_drag();
    Attack attack = Attack::Basic;

    int collision_radius = 8;
    int hp = 10;
    int max_hp = 10;
};

enum Difficulty {
    Normal = 1
};

struct GameState {
    int seed = 1338;
    Difficulty difficulty = Difficulty::Normal;
    int level = 1;

    PlayerState player;

    GameState(int seed, Difficulty difficulty) : seed(seed), difficulty(difficulty) {}
};

struct ProjectileData {
    int damage;
    int radius;
    float force = 2.0f;
    float time_to_live;

    ProjectileData(int damage, int radius, float ttl) : damage(damage), radius(radius), time_to_live(ttl) {}
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

template<typename Spawner> 
void spawn_attack_projectiles(
    const Attack &attack, 
    const FireSettings &fs, 
    const float attack_angle, 
    const Vector2 &gun_exit_position,
    Spawner *s) 
    {
    switch(attack) {
        case Basic:{
                float angle_with_accuracy = attack_angle + RNG::range_f(-fs.accuracy, fs.accuracy);
                Vector2 projectile_velocity = Math::direction_from_angle(angle_with_accuracy) * fs.projectile_speed;
                s->spawn_player_projectile(gun_exit_position, projectile_velocity, fs.p_data);
            }
            break;
        default:
            ASSERT_WITH_MSG(false, "Attack not implemented!");
            break;
    }
}

namespace GameData {
    void game_state_new(int seed, Difficulty difficulty);
    GameState *game_state_get();

    FireSettings create_fire_settings(const Attack &attack, const MapSettings &settings);
    
    /*
    // Create enemy by type and then alter it with current map and gamestate
    EnemySettings create_enemy_settings();
    */
};

#endif