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
    Nailgun = 11,
    Splitter = 12,
    HomingGun = 13,
    Lazer = 14,
    SIZE_OF_Attacks = 15
};

static const char* AttackNames[SIZE_OF_Attacks] = { "Basic", "Double", "Triple", "Circle", "Back", "Flamer", "Rapid", "Side", "Blast", "Boom", "Minigun", "Nailgun", "Splitter", "Homing", "Lazer" };
static const Attack AttackIds[SIZE_OF_Attacks] = { Basic, Double, Triple, Circle, Back, Flamer, Rapid, Side, Blast, Boom, Minigun, Nailgun, Splitter, HomingGun, Lazer };

enum AttackType {
    ProjectileAttack = 0,
    LineAttack = 1
};

struct Attack_t {    
    char *sound_name;
    AttackType type;
    float cooldown; // how much time between projectiles
    float accuracy; // how much the projectile can go of the straight line when fired (spreads in -angle to angle from initial angle)
    
    float knockback; // how far projectiles knock enemies back
    float range; // Time to live so it's range but not really
    float projectile_speed; // speed in pixels / second (x / (1/60)) 
    float projectile_speed_mod; // range that gets added/subtracted from speed
    int projectile_damage;
    int projectile_radius; // for collisions
    int pierce_count;
    int split_count;
    int ammo;
    float homing_radius;
    float projectile_hit_explosion_radius;
    float projectile_death_explosion_radius;
};

static const Attack_t Attacks[SIZE_OF_Attacks] = {
    // sound      | cooldown  | accuracy  | knockback | ttl    | speed  | spd_mod | damage | radius | pierce | split 
    { "basic_fire", ProjectileAttack, 0.25f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,   3,          8,  0, 0, 10, 0, 0, 0         }, // Basic
    { "basic_fire", ProjectileAttack, 0.3f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,  3,          8, 0, 0, 10, 0, 0, 0       }, // Double
    { "basic_fire", ProjectileAttack, 0.35f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,   3,          8, 0, 0, 10, 0, 0, 0     }, // Triple
    { "basic_fire", ProjectileAttack, 0.55f,      2.0f,       2.0f,       0.3f,   bp_spd() / 2, 0,   3,          8,0,0, 10, 0, 0, 0   }, // Circle
    { "basic_fire", ProjectileAttack, 0.3f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,  3,          8, 0,0, 10, 0, 0, 0       }, // Back
    { "basic_fire", ProjectileAttack, 0.08f,      2.0f,       0.0f,       0.25f,   bp_spd(), bp_spd_mod(),  2,8,0,0, 10, 0, 0, 0 }, // Flamer
    { "basic_fire", ProjectileAttack, 0.15f,      4.0f,       2.0f,       0.8f,   bp_spd(), 0,  2,          8,0, 0, 10, 0, 0, 0       }, // Rapid
    { "basic_fire", ProjectileAttack, 0.25f,      8.0f,       2.0f,       0.8f,   bp_spd(), 0,  3,          8,0, 0, 10, 0, 0, 0        }, // Side
    { "basic_fire", ProjectileAttack, 0.5f,     4.0f,       2.0f,       0.3f,   bp_spd(), bp_spd_mod(),  3,          8, 0,0, 10, 0, 0, 0 }, // Blast
    { "basic_fire", ProjectileAttack, 0.7f,      1.0f,       10.0f,       3.0f,   bp_spd() * 0.3f, 0,  9,          16, 0,0, 10, 0, 0, 0 }, // Boom
    { "basic_fire", ProjectileAttack, 0.08f,      12.0f,       1.0f,       0.8f,   bp_spd(), 0,  3,          8, 0, 0, 10, 0, 0, 0     }, // Minigun
    { "basic_fire", ProjectileAttack, 0.14f,      6.0f,       0.0f,       0.8f,   bp_spd(), bp_spd_mod(),  2,          6, 1, 0, 10, 0, 0, 0 }, // Nailgun
    { "basic_fire", ProjectileAttack, 0.25f,      7.0f,       2.0f,       0.8f,   bp_spd(), 0,  2,          8, 0, 4, 10, 0, 0, 0 }, // Splitter
    { "basic_fire", ProjectileAttack, 0.4f,      2.0f,       2.0f,       1.8f,   bp_spd(), 0,  2,          8, 0, 0, 10, 150.0f, 0, 0 }, // Homing
    { "basic_fire", LineAttack, 0.25f,      0.0f,       2.0f,       0.8f,   0, 0,   3,  4,  0, 0, 10, 0, 0, 0 }, // Lazer
};

static const std::vector<float> Projectile_angles[SIZE_OF_Attacks] = { 
    { { 0 }   }, // Basic
    { { -8, 8 } }, // Double
    { { -8, 0, 8 } }, // Triple
    { { 0, 45, 90, 135, 180, 225, 270, 315 } }, // Circle
    { { 0, 180 } }, // Back
    { { 0 } }, // Flamer
    { { 0 } }, // Rapid
    { { 0, 90, -90 } }, // Side
    { { -12, -10, -8, -6, -4, -2, 2, 4, 6, 8, 10, 12 } }, // Blast
    { { 0 } }, // Boom
    { { 0 } }, // Minigun
    { { 0 } }, // Nailgun
    { { 0 } }, // Splitter
    { { 0 } }, // Homing
};

static const std::vector<float> Extra_projectile_angles[SIZE_OF_Attacks] = { 
    { { -8, 8 }   }, // Basic
    { { -8, 0, 8 } }, // Double
    { { -12, -6, 6, 12 } }, // Triple
    { { 0, 45, 90, 135, 180, 225, 270, 315 } }, // Circle  <- extra projectiles does nothing yet
    { { -8, 8, 180 } }, // Back
    { { 4, -4 } }, // Flamer
    { { 2, -2 } }, // Rapid
    { { 8, -8, 90, -90 } }, // Side
    { { -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12 } }, // Blast
    { { 2, -2 } }, // Boom
    { { 2, -2 } }, // Minigun
    { { 1, -1 } }, // Nailgun
    { { 4, -4 } }, // Splitter
    { { 4, -4 } }, // Homing
};

static const std::vector<float> split_angles = { 90, 180, 270, 0, 315, 45, 225, 135 };

/// --------------

const int player_start_hp = 10;
const int player_start_hp_max = 10;
const int enemy_base_hp = 5;
const int enemy_base_hp_max = 5;

struct PlayerStats {
    Attack attack = Attack::Basic;
    int collision_radius = 8;
    float max_velocity = 200.0f;
    int hp = player_start_hp;
    int max_hp = player_start_hp_max;
    float move_acceleration = player_move_acceleration();
    float rotation_speed = player_move_rotation(); // degrees
    float brake_speed = player_drag();
    int ammo_max = 100; // - how much ammo you can maximally have (also start ammo at each level)
    int ammo_recharge = 10; // - how much you recharge every tick
    float ammo_recharge_time = 0.25f; // - how often a tick is
    int shield_max = 0; // ex: 2
    int shield_recharge = 0; // ex: 1
    float shield_recharge_time = 300.0f; // ex: 2.0f 

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

struct ProjectileFireResult {
    float fire_cooldown;
    float knockback;
    char *sound_name;
    int ammo_used;
    bool did_fire;

    ProjectileFireResult(float cooldown, float knockback, char *sound_name) :
        fire_cooldown(cooldown), knockback(knockback), sound_name(sound_name) {
            ammo_used = 0;
            did_fire = false;
    }
};

struct ProjectileSpawn {
    Vector2 position;
    Vector2 last_position;
    float angle;
    float speed;
    int damage;
    int radius;
    float time_to_live;
    int pierce_count;
    int split_count;
    
    float homing_radius;
    float force;
    float explosion_on_death_radius;
    float explosion_on_hit_radius;
    
    bool line = false;
    SDL_Rect line_rect;

    ProjectileSpawn(Vector2 pos, float angle, float speed, int damage, int radius, float ttl, int pierce_count, int split_count) : 
        position(pos),
        angle(angle),
        speed(speed),
        damage(damage), 
        radius(radius), 
        time_to_live(ttl),
        pierce_count(pierce_count),
        split_count(split_count)
    {
        last_position = pos;
        force = 2.0f;
        homing_radius = 0.0f;
        explosion_on_death_radius = 0.0f;
        explosion_on_hit_radius = 0.0f;
    }
};

struct PlayerModifier {
    int attack = NO_ATTACK;
    int collision_radius = 0;
    float max_velocity = 0;
    int hp = 0;
    int max_hp = 0;
	float move_acceleration = 0;
    float rotation_speed = 0; // degrees

    void apply(PlayerStats &player_stats) const {
        if(attack != NO_ATTACK) {
            player_stats.attack = AttackIds[attack];
        }
        player_stats.collision_radius += collision_radius;
        player_stats.max_velocity += max_velocity;
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
    int extra_projectile = 0;
    int split_count = 0;
    float homing_radius = 0;
    float explosion_on_hit_radius = 0;
    float explosion_on_death_radius = 0;
    
    void apply(Attack_t &t_attack) const {
        t_attack.accuracy += accuracy;
        t_attack.cooldown += cooldown;
        t_attack.knockback += knockback;
        t_attack.projectile_damage += projectile_damage;
        t_attack.projectile_radius += projectile_radius;
        t_attack.projectile_speed += projectile_speed;
        t_attack.range += time_to_live;
        t_attack.split_count += split_count;
        t_attack.homing_radius += homing_radius;
        t_attack.projectile_hit_explosion_radius += explosion_on_hit_radius;
        t_attack.projectile_death_explosion_radius += explosion_on_death_radius;
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

    int count_extra_projectiles() {
        int c = 0;
        for(auto &m : projectile_m) {
            c += m.extra_projectile;
        }
        return c;
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
    const std::vector<float> &get_attack_angles(const Attack &attack, const int &projectile_extra_count);
    void split_player_projectile(const MapSettings &settings, const int &count, const Vector2 &position, std::vector<ProjectileSpawn> &projectiles_queue);
    /*
    // Create enemy by type and then alter it with current map and gamestate
    EnemySettings create_enemy();
    */
};

#endif