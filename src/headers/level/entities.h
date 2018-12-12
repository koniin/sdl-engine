#ifndef ENTITIES_H
#define ENTITIES_H

#include "engine.h"
#include "framework.h"
#include "game_data.h"

const Vector2 SHADOW_POSITION = Vector2(22, 22);

struct PlayerInput {
	Vector2 move;
	float fire_x = 0;
	float fire_y = 0;
	float fire_cooldown = 0;
};

struct LifeTime {
    bool marked_for_deletion = false;
    float ttl = 0.0f;
    float time = 0.0f;
};

struct Position {
    Vector2 value;
    Vector2 last;
};

struct Velocity {
    Vector2 value;

    Velocity(): value(Vector2()) {}
    Velocity(Vector2 v): value(v) {}
    Velocity(float xv, float yv): value(xv, yv) {}
};

struct Direction {
    Vector2 value;
    float angle;

    Direction() {
        angle = 0.0f;
    }
};

struct Health {
    int hp = 0;
    int hp_max = 0;
    float invulnerability_timer = 0.0f;
};

struct Damage {
    int value = 0;
    float force = 0; // knockback value ?
    
    // int damage_type;
};

struct CollisionData {
    int radius = 0;
};

struct SpriteComponent {
    float scale;
    float rotation;
    int w, h;
    int16_t radius;
    int16_t color_r;
    int16_t color_g;
    int16_t color_b;
    int16_t color_a;
    size_t sprite_sheet_index;
    std::string sprite_name;
    int layer;
    bool line;
    Vector2 position;

    SpriteComponent() {}

    SpriteComponent(const std::string &sprite_sheet_name, std::string name) : sprite_name(name) {
        sprite_sheet_index = Resources::sprite_sheet_index(sprite_sheet_name);
        auto sprite = Resources::sprite_get_from_sheet(sprite_sheet_index, name);
        w = sprite.w;
        h = sprite.h;
        scale = 1.0f;
        rotation = 1.0f;
        color_r = color_g = color_b = color_a = 255;
        layer = 0;
        line = false;
    }
};

struct TargetConfiguration {

};

struct BlinkEffect {
    float time_to_live = 0;
    float timer = 0;
    float interval = 0;
    float interval_timer = 0;
    std::string original_sprite;
    std::string white_sprite;
};

struct Animation {
    float timer = 0;
    float duration = 0;
    float value = 0;
    float start = 0;
    float end = 0;
    easing_t ease;
    bool enabled = true;

    Animation(){};
    Animation(float duration, float start, float end, easing_t ease): duration(duration), start(start), end(end), ease(ease) {}
};

struct Ammunition {
    int value = 0; // - current ammo
    int max = 0; // - how much ammo you can maximally have (also the start number)
    int recharge = 0; // - how much you recharge every tick
    float recharge_time = 0; // - how often a tick is
    float timer = 0;
};

struct Shield {
    int value = 0; // - current shield
    int max = 0; // - how much is maximum shield value
    int recharge = 0; // - how much you recharge every tick
    float recharge_time = 0; // - how often a tick is
    float timer = 0;
};

struct ChildSprite {
    size_t length = 0;

    std::vector<ECS::Entity> parent;
    std::vector<Position> position;
    std::vector<Vector2> local_position;
    std::vector<SpriteComponent> sprite;
    std::vector<Animation> animation;
    std::vector<Direction> direction;

    void allocate(size_t n) {
        parent.reserve(n);
        position.reserve(n);
        local_position.reserve(n);
        sprite.reserve(n);
        animation.reserve(n);
        direction.reserve(n);
    }

    size_t add(const ECS::Entity &e, const Vector2 &pos, const Vector2 &local_pos, const SpriteComponent &s, const Animation &a, const Direction &d) {
        parent.push_back(e);
        position.push_back({ pos });
        local_position.push_back(local_pos);
        sprite.push_back(s);
        animation.push_back(a);
        direction.push_back(d);

        return length++;
    }

    void remove(size_t i) {
        remove_from(parent, i);
        remove_from(position, i);
        remove_from(local_position, i);
        remove_from(sprite, i);
        remove_from(animation, i);
        remove_from(direction, i);

        --length;
    }

    template<typename T>
    void remove_from(std::vector<T> &v, size_t i) {
        size_t last_index = v.size() - 1;
        ASSERT_WITH_MSG(last_index < v.size(), "Remove is not good to use sometimes yes");
        v[i] = v[last_index];
        v.erase(v.end() - 1);
    }
    
    void clear() {
        length = 0;
        parent.clear();
        position.clear();
        local_position.clear();
        sprite.clear();
        animation.clear();
        direction.clear();
    }
};

struct PlayerConfiguration {
    // Display settings
    float fire_knockback_camera = -6.0f;
    float gun_barrel_distance = 11.0f; // distance from center
    int exhaust_id = 1;
    int shadow_id = 2;

	float rotation_speed; // degrees
	float move_acceleration;
	float max_velocity;
    float brake_speed;
};

struct Player : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<PlayerConfiguration> config;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<Direction> direction;
    std::vector<PlayerInput> input;
    std::vector<SpriteComponent> sprite;
    std::vector<Health> health;
    std::vector<CollisionData> collision;
    std::vector<BlinkEffect> blink;
    std::vector<Ammunition> ammo;
    std::vector<Shield> shield;

    ChildSprite child_sprites;
    std::unordered_map<int, size_t> child_map;
    
    void allocate(size_t n) {
        allocate_entities(n);
        
        initialize(&life_time);
        initialize(&config);
        initialize(&position);
        initialize(&velocity);
        initialize(&direction);
        initialize(&input);
        initialize(&sprite);
        initialize(&health);
        initialize(&collision);
        initialize(&blink);
        initialize(&ammo);
        initialize(&shield);

        child_sprites.allocate(n * 4);
    }

    void clear() {
        child_sprites.clear();
        child_map.clear();

        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(const ECS::Entity &e, const Vector2 &p) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;

        GameState *game_state = GameData::game_state_get();
        PlayerConfiguration pcfg;
        pcfg.max_velocity = game_state->player.max_velocity;
        pcfg.move_acceleration = game_state->player.move_acceleration;
        pcfg.rotation_speed = game_state->player.rotation_speed;
        pcfg.brake_speed = game_state->player.brake_speed;

        config[handle.i] = pcfg;
        position[handle.i] = { p };
        velocity[handle.i] = Velocity();
        direction[handle.i] = Direction();
        input[handle.i] = PlayerInput();
        
        SpriteComponent s = SpriteComponent("shooter", "player_1");
        s.layer = 10;
        sprite[handle.i] = s;
        health[handle.i] = { game_state->player.hp, game_state->player.max_hp };
        collision[handle.i] = { game_state->player.collision_radius };
        blink[handle.i] = BlinkEffect();
        ammo[handle.i] = { game_state->player.ammo_max, game_state->player.ammo_max, game_state->player.ammo_recharge, game_state->player.ammo_recharge_time };

        shield[handle.i] = { game_state->player.shield_max, game_state->player.shield_max, game_state->player.shield_recharge, game_state->player.shield_recharge_time };

        SpriteComponent child_sprite = SpriteComponent("shooter", "bullet_2");
        child_sprite.h = child_sprite.h + (child_sprite.h / 2);
        child_sprite.layer = s.layer - 1;
        auto animation = Animation(0.2f, (float)child_sprite.h, (float)child_sprite.h + 4.0f, easing_sine_in_out);
        Direction d;
        d.value = Vector2::One;
        create_child_sprite(pcfg.exhaust_id, e, 
            p, 
            Vector2(-pcfg.gun_barrel_distance, -pcfg.gun_barrel_distance),
            child_sprite,
            animation,
            d);

        SpriteComponent shadow = SpriteComponent("shooter", "player_1_b");
        shadow.layer = 8;
        auto no_animation = Animation();
        no_animation.enabled = false;
        d.value = Vector2::Zero;
        create_child_sprite(pcfg.shadow_id, e, 
            p, 
            SHADOW_POSITION,
            shadow,
            no_animation,
            d);
    }

    void create_child_sprite(int id, const ECS::Entity &e, const Vector2 &pos, const Vector2 &local_pos, const SpriteComponent &s, const Animation &a, const Direction &d) {
        size_t new_sprite_id = child_sprites.add(e, 
            pos, 
            local_pos,
            s,
            a,
            d);
            
        child_map[id] = new_sprite_id;
    }

    size_t get_child_sprite_index(int id) {
        return child_map[id];
    }
};

struct Pierce {
    int count = 0;
    int limit = 0;
};

struct Split {
    int count = 0;
};

struct HomingComponent {
    float radius = 0;
    bool has_target = false;
    ECS::Entity target;
};

struct OnDeath {
    bool explosion;
    float explosion_radius = 0.0f;
};

struct DamageOnHit {
    float explosion_radius = 0.0f;
};

struct Projectile : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<SpriteComponent> sprite;
    std::vector<Damage> damage;
    std::vector<CollisionData> collision;
    std::vector<Pierce> pierce;
    std::vector<Split> split;
    std::vector<HomingComponent> homing;
    std::vector<OnDeath> on_death;
    std::vector<DamageOnHit> on_hit;

    void allocate(size_t n) {
        allocate_entities(n);

        initialize(&life_time);
        initialize(&position);
        initialize(&velocity);
        initialize(&sprite);
        initialize(&damage);
        initialize(&collision);
        initialize(&pierce);
        initialize(&split);
        initialize(&homing);
        initialize(&on_death);
        initialize(&on_hit);
    }

    void clear() {
        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(ECS::Entity e, const ProjectileSpawn &p, const SpriteComponent &s) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;
        life_time[handle.i].ttl = p.time_to_live;
        life_time[handle.i].time = 0;
        position[handle.i] = { p.position, p.last_position };
        velocity[handle.i] = Velocity(Math::direction_from_angle(p.angle) * p.speed);
        sprite[handle.i] = s;
        damage[handle.i] = { p.damage, p.force };
        collision[handle.i] = { p.radius };
        pierce[handle.i] = { 0, p.pierce_count };
        split[handle.i] = { p.split_count };
        homing[handle.i] = { p.homing_radius, false };
        on_death[handle.i] = { p.explosion_on_death_radius > 0 };
        on_hit[handle.i] = { p.explosion_on_hit_radius };
    }
};

struct AIComponent {
    float fire_range = 0.0f;
    float engagement_range = 0.0f;
    float target_min_range = 0.0f;
    float acceleration = 0.0f;
    bool has_target = false;
    float fire_cooldown = 0.0f;
    bool activated = false;    
};

struct Target : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<TargetConfiguration> config;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<Direction> direction;
    std::vector<SpriteComponent> sprite;
    std::vector<BlinkEffect> blink;
    std::vector<Health> health;
    std::vector<CollisionData> collision;
    std::vector<AIComponent> ai;
    std::vector<TargetWeaponConfiguration> weapon;

    ChildSprite child_sprites;
    std::unordered_map<int, size_t> child_map;
    
    void allocate(size_t n) {
        allocate_entities(n);

        initialize(&life_time);
        initialize(&config);
        initialize(&position);
        initialize(&velocity);
        initialize(&direction);
        initialize(&sprite);
        initialize(&blink);
        initialize(&health);
        initialize(&collision);
        initialize(&ai);
        initialize(&weapon);
        
        child_sprites.allocate(n * 2);
    }

    void clear() {
        child_sprites.clear();
        child_map.clear();

        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(const ECS::Entity &e, const Vector2 &p, const Enemy &enemy) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;

        config[handle.i] = TargetConfiguration();
        position[handle.i] = { p };
        velocity[handle.i] = Velocity(0, 0);
        direction[handle.i] = Direction();
        SpriteComponent s = SpriteComponent("shooter", "enemy_1");
        s.layer = 10;
        sprite[handle.i] = s;
        blink[handle.i] = BlinkEffect();
        health[handle.i] = { enemy.hp, enemy.max_hp };
        collision[handle.i] = { enemy.collision_radius };
        ai[handle.i] = { enemy.activation_radius, 400.0f, 60000.0f, 10.0f }; // 10 acceleration is kind ok sluggish
        weapon[handle.i] = enemy.weapon;

        SpriteComponent shadow = SpriteComponent("shooter", "enemy_1_b");
        shadow.layer = 8;
        auto no_animation = Animation();
        no_animation.enabled = false;
        Direction d;
        d.value = Vector2::Zero;
        create_child_sprite(e.id, e,
            p, 
            SHADOW_POSITION,
            shadow,
            no_animation,
            d);
    }
    
    void create_child_sprite(int id, const ECS::Entity &e, const Vector2 &pos, const Vector2 &local_pos, const SpriteComponent &s, const Animation &a, const Direction &d) {
        size_t new_sprite_id = child_sprites.add(e, 
            pos, 
            local_pos,
            s,
            a,
            d);
            
        child_map[id] = new_sprite_id;
    }

    size_t get_child_sprite_index(int id) {
        return child_map[id];
    }
};

struct Effect;
struct EffectModifer;
typedef void (*effect_modifier)(Effect &effects, const int &i, const std::string &modifier_data_s);

struct EffectData {
    float time_to_live;
    float timer;
    
    // follow
    bool has_target = false;
    ECS::Entity follow;
    Vector2 local_position;

    // frame_counter effects
    bool modifier_enabled;
    float modifier_time;
    effect_modifier modifier;
    int modifier_data_i;
    std::string modifier_data_s;

    EffectData(){};
    EffectData(float ttl) : time_to_live(ttl) {
        timer = 0;
        modifier_time = 0.0f;
        modifier_enabled = false;
    }
};

struct Effect : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<SpriteComponent> sprite;
    std::vector<EffectData> effect;

    struct SpawnEffect {
        Position position;
        Velocity velocity;
        SpriteComponent sprite;
        EffectData effect;
    };
    std::vector<SpawnEffect> effect_queue;

    void allocate(size_t n) {
        allocate_entities(n);
    
        initialize(&life_time);
        initialize(&position);
        initialize(&velocity);
        initialize(&sprite);
        initialize(&effect);
    }

    void clear() {
        effect_queue.clear();

        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(ECS::Entity &e, const Position &p, const Velocity &v, const SpriteComponent &s, const EffectData &ef) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;
        
        position[handle.i] = p;
        velocity[handle.i] = v;
        sprite[handle.i] = s;
        effect[handle.i] = ef;
    }
};

struct Drop : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<SpriteComponent> sprite;
    std::vector<CollisionData> collision;
    
    void allocate(size_t n) {
        allocate_entities(n);

        initialize(&life_time);
        initialize(&position);
        initialize(&velocity);
        initialize(&sprite);
        initialize(&collision);
    }

    void clear() {
        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(ECS::Entity e, const ProjectileSpawn &p, const SpriteComponent &s) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;
        life_time[handle.i].ttl = 0;
        life_time[handle.i].time = 0;
        position[handle.i] = { p.position, p.last_position };
        velocity[handle.i] = Velocity(Math::direction_from_angle(p.angle) * p.speed);
        sprite[handle.i] = s;
        collision[handle.i] = { p.radius };
    }
};

inline void sprite_effect(Effect &effects, const int &i, const std::string &modifier_data_s) {
    Engine::logn("sprite change");
    effects.sprite[i].sprite_name = modifier_data_s;
}

template<typename T>
Position &get_position(T &entity_data, ECS::Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    return entity_data.position[handle.i];
}

template<typename T>
Velocity &get_velocity(T &entity_data, ECS::Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    return entity_data.velocity[handle.i];
}

template<typename T>
Health &get_health(T &entity_data, ECS::Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    return entity_data.health[handle.i];
}

template<typename T>
Shield &get_shield(T &entity_data, ECS::Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    return entity_data.shield[handle.i];
}

template<typename T>
Damage &get_damage(T &entity_data, ECS::Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    return entity_data.damage[handle.i];
}

template<typename T>
void set_position(T &entity_data, ECS::Entity e, Position p) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    entity_data.position[handle.i] = p;
}

template<typename T>
void set_velocity(T &entity_data, ECS::Entity e, Velocity v) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    entity_data.velocity[handle.i] = v;
}

template<typename T>
void set_sprite(T &entity_data, ECS::Entity e, SpriteComponent s) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    entity_data.sprite[handle.i] = s;
}

template<typename T>
void set_health(T &entity_data, ECS::Entity e, const Health h) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    entity_data.health[handle.i] = h;
}

template<typename T>
void set_damage(T &entity_data, ECS::Entity e, const Damage d) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive or fetching from wrong entity");
    auto handle = entity_data.get_handle(e);
    entity_data.damage[handle.i] = d;
}

template<typename T>
void mark_for_deletion(T &entity_data, ECS::Entity e) {
    auto handle = entity_data.get_handle(e);
    if(entity_data.is_valid(handle))
        entity_data.life_time[handle.i].marked_for_deletion = true;
}

// time in seconds
inline void set_invulnerable(Health &health, const float &time) {
    health.invulnerability_timer = time * Time::delta_time_fixed * 60.0f;
}

template<typename T>
bool is_invulnerable(T &entity, ECS::Entity e) {
    auto &health = get_health(entity, e);
    return health.invulnerability_timer > 0.0f;
}

inline int deal_damage(Projectile &projectile, ECS::Entity projectile_entity, Target &target, ECS::Entity target_entity) {
    auto &damage = get_damage(projectile, projectile_entity);
    auto &health = get_health(target, target_entity);
    health.hp -= damage.value;
    Engine::logn("Deal damage to target: %d", damage.value);
    return damage.value;
}

inline int deal_damage(Projectile &projectile, ECS::Entity projectile_entity, Player &player, ECS::Entity player_entity) {
    auto &damage = get_damage(projectile, projectile_entity);
    auto &health = get_health(player, player_entity);
    auto &shield = get_shield(player, player_entity);
    // Engine::logn("\n\tincoming damage: %d", damage.value);
    // Engine::logn("\tshield: %d", shield.value);
    int damage_after_shield = Math::max_i(0, damage.value - shield.value);
    // Engine::logn("\tdamage after shield: %d", damage_after_shield);
    shield.value = Math::max_i(0, shield.value - damage.value);
    // Engine::logn("\tshield after damage: %d", shield.value);
    health.hp -= damage_after_shield;
    // Engine::logn("\tDeal damage to player: %d", damage_after_shield);
    return damage_after_shield;
}

template<typename T>
void blink_sprite(T &entity_data, ECS::Entity e, float ttl, float interval) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);

    if(entity_data.blink[handle.i].timer > 0)
        return;

    BlinkEffect b;
    b.time_to_live = ttl;
    b.interval = interval;
    b.original_sprite = entity_data.sprite[handle.i].sprite_name;
    b.white_sprite = entity_data.sprite[handle.i].sprite_name + "_w";
    //b.original_sheet = entity_data.sprite[handle.i].sprite_sheet_index;
    // We assume the next sheet is the white version
    //b.white_sheet = entity_data.sprite[handle.i].sprite_sheet_index + 1;
    entity_data.sprite[handle.i].sprite_name = b.white_sprite;
    entity_data.blink[handle.i] = b;
}
#endif