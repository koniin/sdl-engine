#ifndef ENTITIES_H
#define ENTITIES_H

#include "engine.h"
#include "framework.h"

struct PlayerInput {
	// Input
	float move_x;
	float move_y;
	float fire_x;
	float fire_y;
	float fire_cooldown;
	bool shield;

    PlayerInput() {
        fire_cooldown = 0.0f;
        shield = false;
    }
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

    size_t add(const ECS::Entity &p, const Vector2 &pos, const Vector2 &local_pos, const SpriteComponent &s, const Animation &a) {
        parent.push_back(p);
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
    Position *position;
    Velocity *velocity;
    Direction *direction;
    PlayerInput *input;
    SpriteComponent *sprite;
    
    ChildSprite child_sprites;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        direction = new Direction[n];
        input = new PlayerInput[n];
        sprite = new SpriteComponent[n];

        allocate_entities(n, 5);

        add(position);
        add(velocity);
        add(direction);
        add(input);
        add(sprite);

        child_sprites.allocate(16);
    }
};

struct Projectile : ECS::EntityData {
    Position *position;
    Velocity *velocity;
    SpriteComponent *sprite;

    const int radius = 8;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        sprite = new SpriteComponent[n];

        allocate_entities(n, 3);

        add(position);
        add(velocity);
        add(sprite);
    }
};

struct BlinkEffect {
    int frames_to_live = 0;
    int frame_counter = 0;
    int interval;
    size_t original_sheet;
    size_t white_sheet;
};

struct Target : ECS::EntityData {
    Position *position;
    Velocity *velocity;
    SpriteComponent *sprite;
    BlinkEffect* blink;

    const int radius = 8;

    // Here you can put a list of base data for different targets

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        sprite = new SpriteComponent[n];
        blink = new BlinkEffect[n];

        allocate_entities(n, 4);

        add(position);
        add(velocity);
        add(sprite);
        add(blink);
    }
};

struct Effect;
struct EffectModifer;
typedef void (*effect_modifier)(const Effect &effects, const int &i, const std::string &modifier_data_s);

struct EffectData {
    int frames_to_live;
    int frame_counter;
    
    // follow
    bool has_target = false;
    ECS::Entity follow;
    Vector2 local_position;

    // frame_counter effects
    bool modifier_enabled;
    int modifier_frame; 
    effect_modifier modifier;
    int modifier_data_i;
    std::string modifier_data_s;

    EffectData(){};
    EffectData(int frames) : frames_to_live(frames) {
        frame_counter = 0;
    }
};

struct Effect : ECS::EntityData {
    Position *position;
    Velocity *velocity;
    SpriteComponent *sprite;
    EffectData *effect;

    const int effect_layer = 2;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        sprite = new SpriteComponent[n];
        effect = new EffectData[n];

        allocate_entities(n, 4);

        add(position);
        add(velocity);
        add(sprite);
        add(effect);
    }
};

void sprite_effect(const Effect &effects, const int &i, const std::string &modifier_data_s) {
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

#endif