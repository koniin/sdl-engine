#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "engine.h"

// Pixels per frame
// Base projectile speed
constexpr float bp_spd() {
    return 8.0f / 0.016667f;
}

constexpr float bp_spd_mod() {
    return 1.0f / 0.016667f;
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

struct Upgrade;
struct PlayerModifier;
struct ProjectileStatModifier;
struct EnemyModifier;
struct Attack_t;
struct Enemy;

/// --------------
/// Attacks

const int NO_ATTACK = 666;

enum Attack {
    Basic = 0,
    Double = 1,
    Triple = 2,
    Circle = 3,
    Back = 4,
    Flamer = 5,
    Rapid = 6,
    Side = 7,
    Blast = 8,
    Boom = 9,
    Minigun = 10,
    SIZE_OF_Attacks = 11
};

static const char* AttackNames[SIZE_OF_Attacks] = { "Basic", "Double", "Triple", "Circle", "Back", "Flamer", "Rapid", "Side", "Blast", "Boom", "Minigun" };
static const Attack AttackIds[SIZE_OF_Attacks] = { Basic, Double, Triple, Circle, Back, Flamer, Rapid, Side, Blast, Boom, Minigun };

struct Attack_t {    
    char *sound_name;
    float cooldown; // how much time between projectiles
    float accuracy; // how much the projectile can go of the straight line when fired (spreads in -angle to angle from initial angle)
    
    float knockback; // how far projectiles knock enemies back
    float range; // Time to live so it's range but not really
    float projectile_speed; // speed in pixels / second (x / (1/60)) 
    float projectile_speed_mod;
    int projectile_damage;
    int projectile_radius; // for collisions

    std::vector<float> projectile_angles; // how many projectiles and their angle offset from direction angle
};

static const Attack_t Attacks[SIZE_OF_Attacks] = {
    // sound      | cooldown  | accuracy  | knockback | ttl    | speed  | spd_mod | damage    | radius | angles   
    { "basic_fire", 0.25f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,   3,          8,        { 0 }   }, // Basic
    { "basic_fire", 0.3f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,  3,          8,        { -8, 8 } }, // Double
    { "basic_fire", 0.35f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,   3,          8,        { -8, 0, 8 } }, // Triple
    { "basic_fire", 0.55f,      2.0f,       2.0f,       0.3f,   bp_spd() / 2, 0,   3,          8,  { 0, 45, 90, 135, 180, 225, 270, 315 } }, // Circle
    { "basic_fire", 0.3f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,  3,          8,        { 0, 180 } }, // Back
    { "basic_fire", 0.08f,      2.0f,       0.0f,       0.25f,   bp_spd(), bp_spd_mod(),  2,          8,        { 0 } }, // Flamer
    { "basic_fire", 0.15f,      4.0f,       2.0f,       0.8f,   bp_spd(), 0,  2,          8,        { 0 } }, // Rapid
    { "basic_fire", 0.25f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,  3,          8,        { 0, 90, -90 } }, // Side
    { "basic_fire", 0.5f,     4.0f,       2.0f,       0.3f,   bp_spd(), bp_spd_mod(),  3,          8,        { -12, -10, -8, -6, -4, -2, 2, 4, 6, 8, 10, 12 } }, // Blast
    { "basic_fire", 0.7f,      1.0f,       10.0f,       3.0f,   bp_spd() * 0.3f, 0,  9,          16,        { 0 } }, // Boom
    { "basic_fire", 0.08f,      12.0f,       1.0f,       0.8f,   bp_spd(), 0,  3,          8,        { 0 } }, // Minigun
};

/// --------------

const int player_start_hp = 10;
const int player_start_hp_max = 10;
const int enemy_base_hp = 5;
const int enemy_base_hp_max = 5;

struct PlayerStats {
    Attack attack = Attack::Flamer;
    int collision_radius = 8;
    float drag = player_drag();
    int hp = player_start_hp;
    int max_hp = player_start_hp_max;
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
    int hp = enemy_base_hp;
    int max_hp = enemy_base_hp_max;
    int collision_radius = 8;
    float activation_radius = 100.0f;

    TargetWeaponConfiguration weapon;
};

struct EnemyModifier {
    int hp = 0;
    int max_hp = 0;
    int collision_radius = 0;
    float activation_radius = 0;
    
    void apply(Enemy &enemy) const {
        enemy.hp += hp;
        enemy.max_hp += max_hp;
        enemy.collision_radius += collision_radius;
        enemy.activation_radius += activation_radius;
    }
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

    // void reverse(PlayerStats &player_stats) const {
    //     player_stats.collision_radius -= collision_radius;
    //     player_stats.drag -= drag;
    //     player_stats.move_acceleration -= move_acceleration;
    //     player_stats.rotation_speed -= rotation_speed;
    // }
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

struct MapModifier {
    char *name;
    char *description;
    float drop_modifier;
    
    std::vector<PlayerModifier> player_m;
    std::vector<EnemyModifier> enemy_m;
    std::vector<ProjectileStatModifier> player_proj_m;
    std::vector<ProjectileStatModifier> target_proj_m;

    void apply_player_projectile_modifiers(Attack_t &t_attack) const {
        for(auto &m : player_proj_m) {
            m.apply(t_attack);
        }
    }

    void apply_enemy_modifiers(Enemy &enemy) const {
        for(auto &m : enemy_m) {
            m.apply(enemy);
        }
    }

    // void apply_enemy_projectile_modifiers(Attack_t &t_attack) const {
    //     for(auto &m : target_proj_m) {
    //         m.apply(t_attack);
    //     }
    // }
};

struct MapSettings {
    MapSize map_size;
    MapStyle style;
    std::vector<MapModifier> modifiers;

    void apply_player_projectile_modifiers(Attack_t &t_attack) const {
        for(auto &m : modifiers) {
            m.apply_player_projectile_modifiers(t_attack);
        }
    }

    void apply_enemy_modifiers(Enemy &enemy) const {
        for(auto &m : modifiers) {
            m.apply_enemy_modifiers(enemy);
        }
    }
};

namespace GameData {
    void game_state_new(int seed, Difficulty difficulty);
    GameState *game_state_get();
    void load_data();
    std::vector<Upgrade> &get_upgrades();
    std::vector<MapModifier> &get_map_modifiers();
    void set_attack(const Attack &attack);
    void add_upgrade(const Upgrade &u);
    FireSettings trigger_projectile_fire(const Attack &attack, const MapSettings &settings, float angle, Vector2 pos, std::vector<ProjectileSpawn> &projectiles_queue);

    /*
    // Create enemy by type and then alter it with current map and gamestate
    EnemySettings create_enemy();
    */
};

#endif