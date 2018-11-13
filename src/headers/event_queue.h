
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "entities.h"

inline void queue_projectile(Projectile &entity_data, Vector2 p, Vector2 v) {
    entity_data.projectile_queue.push_back({ p , v });
}

void spawn_projectile(Projectile &entity_data, Vector2 p, Vector2 v);

inline void spawn_projectiles(Projectile &entity_data) {
    for(size_t i = 0; i < entity_data.projectile_queue.size(); i++) {
        spawn_projectile(entity_data, entity_data.projectile_queue[i].position, entity_data.projectile_queue[i].velocity);
    }
    entity_data.projectile_queue.clear();
}

//////////////////////////////////

struct SpawnEffect {
    Position position;
    Velocity velocity;
    SpriteComponent sprite;
    EffectData effect;
};
static std::vector<SpawnEffect> effect_queue;

void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent) {
    auto spr = SpriteComponent("shooter", "bullet_1.png");
    spr.layer = 2;
    auto effect = EffectData(2 * Time::delta_time_fixed);
    effect.follow = parent;
    effect.local_position = local_position;
    effect.has_target = true;
    effect_queue.push_back({ p, Velocity(), spr, effect });
}

void spawn_explosion(Vector2 position, float offset_x, float offset_y) {
    auto spr = SpriteComponent("shooter", "explosion_1.png");
    spr.layer = 0;
    auto effect = EffectData(4 * Time::delta_time_fixed);
    effect.modifier_enabled = true;
    effect.modifier_data_s = "explosion_2.png";
    effect.modifier_time = 2 * Time::delta_time_fixed;
    effect.modifier = sprite_effect;
    Vector2 blast_position = position;
    blast_position.x += RNG::range_f(-offset_x, offset_x);
    blast_position.y += RNG::range_f(-offset_y, offset_y);
    effect_queue.push_back({ { blast_position }, Velocity(), spr, effect });
}

void spawn_effect(const Position p, const Velocity v, const SpriteComponent s, const EffectData ef);

void spawn_effects() {
    for(size_t i = 0; i < effect_queue.size(); i++) {
        spawn_effect(effect_queue[i].position, effect_queue[i].velocity, effect_queue[i].sprite, effect_queue[i].effect);
    }
    effect_queue.clear();
}

#endif