#ifndef ENTITIES_H
#define ENTITIES_H

#include "engine.h"
#include "framework.h"


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

struct PlayerConfiguration {
    int16_t radius = 8;
	float rotation_speed = player_move_rotation(); // degrees
	float move_acceleration = player_move_acceleration();
	float drag = player_drag();
    float gun_barrel_distance = 11.0f; // distance from center
    float fire_knockback = 2.0f; // pixels
    float fire_knockback_camera = -6.0f;
    int exhaust_id = 1;
};

struct PlayerInput {
	// Input
	float move_x = 0;
	float move_y = 0;
	float fire_x = 0;
	float fire_y = 0;
	float fire_cooldown = 0;
};

struct LifeTime {
    bool marked_for_deletion = false;
};

struct Position {
    Vector2 value;
    Vector2 last;
};

struct Velocity {
    Vector2 value;

    Velocity(): value(Vector2()) {}
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

// Pixels per frame
constexpr float player_projectile_speed() {
    return 8.0f / 0.016667f;
}
// Remember no fancy stuff, just plain data
struct WeaponConfgiruation {
    float fire_cooldown = 0.15f; // s
	float projectile_speed = player_projectile_speed();
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
    }
};

struct TargetConfiguration {

};

struct BlinkEffect {
    float time_to_live = 0;
    float timer = 0;
    float interval = 0;
    float interval_timer = 0;
    size_t original_sheet;
    size_t white_sheet;
};

struct AI {
    float search_area = 0.0f;
    bool has_target = false;
    float fire_cooldown = 0.0f;
};

struct Animation {
    float timer = 0;
    float duration = 0;
    float value = 0;
    float start = 0;
    float end = 0;
    easing_t ease;

    Animation(){};
    Animation(float duration, float start, float end, easing_t ease): duration(duration), start(start), end(end), ease(ease) {}
};

struct ChildSprite {
    size_t length = 0;

    std::vector<ECS::Entity> parent;
    std::vector<Position> position;
    std::vector<Vector2> local_position;
    std::vector<SpriteComponent> sprite;
    std::vector<Animation> animation;

    void allocate(size_t n) {
        parent.reserve(n);
        position.reserve(n);
        local_position.reserve(n);
        sprite.reserve(n);
        animation.reserve(n);
    }

    size_t add(const ECS::Entity &e, const Vector2 &pos, const Vector2 &local_pos, const SpriteComponent &s, const Animation &a) {
        parent.push_back(e);
        position.push_back({ pos });
        local_position.push_back(local_pos);
        sprite.push_back(s);
        animation.push_back(a);

        return length++;
    }

    void remove(size_t i) {
        remove_from(parent, i);
        remove_from(position, i);
        remove_from(local_position, i);
        remove_from(sprite, i);
        remove_from(animation, i);

        --length;
    }

    template<typename T>
    void remove_from(std::vector<T> &v, size_t i) {
        size_t last_index = v.size() - 1;
        v[i] = v[last_index];
        v.erase(v.end() - 1);
    }
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
    std::vector<WeaponConfgiruation> weapon;
    std::vector<BlinkEffect> blink;

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
        initialize(&weapon);
        initialize(&blink);

        child_sprites.allocate(16);
    }

    void clear() {
        child_sprites.length = 0;
        child_map.clear();

        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(const ECS::Entity &e, const Vector2 &p) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;

        PlayerConfiguration pcfg;
        config[handle.i] = pcfg;
        position[handle.i] = { p };
        velocity[handle.i] = Velocity();
        direction[handle.i] = Direction();
        input[handle.i] = PlayerInput();
        
        SpriteComponent s = SpriteComponent("shooter", "player_1.png");
        s.layer = 1;
        sprite[handle.i] = s;
        health[handle.i] = { 10, 10 };
        collision[handle.i] = { 8 };
        weapon[handle.i] = WeaponConfgiruation();
        blink[handle.i] = BlinkEffect();

        SpriteComponent child_sprite = SpriteComponent("shooter", "bullet_2.png");
        child_sprite.h = child_sprite.h + (child_sprite.h / 2);
        child_sprite.layer = 0;
        auto animation = Animation(0.2f, (float)child_sprite.h, (float)child_sprite.h + 4.0f, easing_sine_in_out);
        create_child_sprite(pcfg.exhaust_id, e, 
            p, 
            Vector2(-pcfg.gun_barrel_distance, -pcfg.gun_barrel_distance),
            child_sprite,
            animation);
    
    }

    void create_child_sprite(int id, const ECS::Entity &e, const Vector2 &pos, const Vector2 &local_pos, const SpriteComponent &s, const Animation &a) {
        size_t new_sprite_id = child_sprites.add(e, 
            pos, 
            local_pos,
            s,
            a);

        child_map[id] = new_sprite_id;
    }

    size_t get_child_sprite_index(int id) {
        return child_map[id];
    }
};

struct Projectile : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<SpriteComponent> sprite;
    std::vector<Damage> damage;
    std::vector<CollisionData> collision;

    struct SpawnProjectile {
        Vector2 position;
        Vector2 velocity;
    };
    std::vector<SpawnProjectile> projectile_queue;

    void allocate(size_t n) {
        allocate_entities(n);

        initialize(&life_time);
        initialize(&position);
        initialize(&velocity);
        initialize(&sprite);
        initialize(&damage);
        initialize(&collision);

        projectile_queue.reserve(64);
    }

    void clear() {
        projectile_queue.clear();

        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void queue_projectile(Vector2 p, Vector2 v) {
        projectile_queue.push_back({ p , v });
    }

    void create(ECS::Entity e, Vector2 p, Vector2 v) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;

        position[handle.i] = { p, p };
        velocity[handle.i] = Velocity(v.x, v.y);
        SpriteComponent s = SpriteComponent("shooter", "bullet_2.png");
        sprite[handle.i] = s;
        damage[handle.i] = { 1, 2.0f };
        collision[handle.i] = { 8 };
    }
};

struct Target : ECS::EntityData {
    std::vector<LifeTime> life_time;
    std::vector<TargetConfiguration> config;
    std::vector<Position> position;
    std::vector<Velocity> velocity;
    std::vector<SpriteComponent> sprite;
    std::vector<BlinkEffect> blink;
    std::vector<Health> health;
    std::vector<CollisionData> collision;
    std::vector<AI> ai;
    std::vector<WeaponConfgiruation> weapon;

    void allocate(size_t n) {
        allocate_entities(n);

        initialize(&life_time);
        initialize(&config);
        initialize(&position);
        initialize(&velocity);
        initialize(&sprite);
        initialize(&blink);
        initialize(&health);
        initialize(&collision);
        initialize(&ai);
        initialize(&weapon);
    }

    void clear() {
        for(int i = 0; i < length; i++) {
            remove(entity[i]);
        }
    }

    void create(const ECS::Entity &e, const Vector2 &p) {
        add_entity(e);
        auto handle = get_handle(e);
        life_time[handle.i].marked_for_deletion = false;

        config[handle.i] = TargetConfiguration();
        position[handle.i] = { p };
        velocity[handle.i] = Velocity(0, 0);
        SpriteComponent s = SpriteComponent("shooter", "enemy_1.png");
        s.layer = 1;
        sprite[handle.i] = s;
        blink[handle.i] = BlinkEffect();
        health[handle.i] = { 2, 2 };
        collision[handle.i] = { 8 };
        ai[handle.i] = { 100.0f };
        weapon[handle.i] = WeaponConfgiruation();
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
    health.hp -= damage.value;
    Engine::logn("Deal damage to player: %d", damage.value);
    return damage.value;
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
    b.original_sheet = entity_data.sprite[handle.i].sprite_sheet_index;
    // We assume the next sheet is the white version
    b.white_sheet = entity_data.sprite[handle.i].sprite_sheet_index + 1;
    entity_data.sprite[handle.i].sprite_sheet_index = b.white_sheet;
    entity_data.blink[handle.i] = b;
}
#endif