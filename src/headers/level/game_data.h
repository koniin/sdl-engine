#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "engine.h"

// Pixels per frame
// Base projectile speed
constexpr float bp_spd() {
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

inline float per_frame_calculation(float s) {
    return s / 0.016667f;
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

/// --------------
/// Attacks

const int NO_ATTACK = 666;

enum Attack {
    Basic = 0,
    SIZE_OF_Attacks = 1
};

static const char* AttackNames[SIZE_OF_Attacks] = { "Basic" };
static const Attack AttackIds[SIZE_OF_Attacks] = { Basic };
static_assert(sizeof(AttackNames)/sizeof(char*) == Attack::SIZE_OF_Attacks, "AttackNames sizes dont match");
static_assert(sizeof(AttackIds)/sizeof(char*) == Attack::SIZE_OF_Attacks, "AttackIds sizes dont match");

struct Attack_t {    
    char *sound_name;
    float cooldown; // how much time between projectiles
    float accuracy; // how much the projectile can go of the straight line when fired (spreads in -angle to angle from initial angle)
    
    float knockback;
    float range; // Time to live so it's range but not really
    float projectile_speed; 
    int projectile_damage;
    int projectile_radius; // for collisions
};

static const Attack_t Attacks[SIZE_OF_Attacks] = {
    // sound      | cooldown  | accuracy  | knockback | range | speed     | damage    | radius    
    { "basic_fire", 0.25f,      8.0f,       2.0f,       0.8f,   bp_spd(),   1,          8 }
};

/// --------------

struct Upgrade;

struct PlayerStats {
    Attack attack = Attack::Basic;
    int collision_radius = 8;
    float drag = player_drag();
    int hp = 10;
    int max_hp = 10;
    float move_acceleration = player_move_acceleration();
    float rotation_speed = player_move_rotation(); // degrees
	
    void increase_hp(int amount) {
        hp = Math::clamp_i(hp + amount, 0, max_hp);
    }
};

struct TargetWeaponConfiguration {
    float fire_cooldown = 0.5f;
    float projectile_speed = 6.0f / 0.016667f;
};

struct Enemy {
    int hp = 2;
    int max_hp = 2;
    int collision_radius = 8;
    float activation_radius = 100.0f;

    TargetWeaponConfiguration weapon;
};

enum Difficulty {
    Normal = 1
};

struct GameState {
    int seed = 1338;
    Difficulty difficulty = Difficulty::Normal;
    int level = 1;

    PlayerStats player;
	std::vector<Upgrade> player_upgrades;

    GameState(int seed, Difficulty difficulty) : seed(seed), difficulty(difficulty) {}
};

struct FireSettings {
    float fire_cooldown;
    float knockback;
    char *sound_name;

    FireSettings(float cooldown, float knockback, char *sound_name) :
        fire_cooldown(cooldown), knockback(knockback), sound_name(sound_name) {
    }
};

struct ProjectileSpawn {
    Vector2 position;
    float angle;
    float speed;
    int damage;
    int radius;
    float time_to_live;

    float force;

    ProjectileSpawn(Vector2 pos, float angle, float speed, int damage, int radius, float ttl) : 
        position(pos),
        angle(angle),
        speed(speed),
        damage(damage), 
        radius(radius), 
        time_to_live(ttl) 
    {
        force = 2.0f;
    }
};

struct PlayerModifier {
    int attack = NO_ATTACK;
    int collision_radius = 0;
    float drag = 0;
    int hp = 0;
    int max_hp = 0;
	float move_acceleration = 0;
    float rotation_speed = 0; // degrees

    void apply(PlayerStats &player_stats) const {
        if(attack != NO_ATTACK) {
            player_stats.attack = AttackIds[attack];
        }
        player_stats.collision_radius += collision_radius;
        player_stats.drag += drag;
        player_stats.max_hp += max_hp;
        player_stats.increase_hp(hp);
        player_stats.move_acceleration += move_acceleration;
        player_stats.rotation_speed += rotation_speed;
    }
};

struct ProjectileStatModifier {
    float accuracy = 0; // how much the projectile can go of the straight line when fired (spreads in -angle to angle from initial angle)
    float cooldown = 0; // how much time between projectiles
    float knockback = 0;
    int projectile_damage = 0;
    int projectile_radius = 0; // for collisions
    float projectile_speed = 0; 
    float time_to_live = 0; // Time to live so it's range but not really
    
    void apply(Attack_t &t_attack) const {
        t_attack.accuracy += accuracy;
        t_attack.cooldown += cooldown;
        t_attack.knockback += knockback;
        t_attack.projectile_damage += projectile_damage;
        t_attack.projectile_radius += projectile_radius;
        t_attack.projectile_speed += projectile_speed;
        t_attack.range += time_to_live;
    }
};

struct Upgrade {
    char *name;
    char *description;
    std::vector<PlayerModifier> player_m;
    std::vector<ProjectileStatModifier> projectile_m;
    
    void apply_projectile_modifiers(Attack_t &t_attack) const {
        for(auto &m : projectile_m) {
            m.apply(t_attack);
        }
    }

    void apply_player_modifiers(PlayerStats &player_stats) const {
        for(auto &m : player_m) {
            m.apply(player_stats);
        }
    }
};

namespace GameData {
    void game_state_new(int seed, Difficulty difficulty);
    GameState *game_state_get();
    void load_upgrades();
    std::vector<Upgrade> &get_upgrades();
    void set_attack(const Attack &attack);
    void add_upgrade(const Upgrade &u);
    FireSettings trigger_projectile_fire(const Attack &attack, const MapSettings &settings, float angle, Vector2 pos, std::vector<ProjectileSpawn> &projectiles_queue);

    /*
    // Create enemy by type and then alter it with current map and gamestate
    EnemySettings create_enemy();
    */
};

#endif