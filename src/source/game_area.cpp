#include "level\game_area.h"

void GameArea::load(const Rectangle &bounds) {
    world_bounds = bounds;
}

void GameArea::spawn_player(Vector2 position) {
    auto e = entity_manager.create();
    players.create(e, position);
}

void GameArea::spawn_target(Vector2 position) {
    auto e = entity_manager.create();
    targets.create(e, position);
}

void GameArea::spawn_projectiles() {
    for(size_t i = 0; i < projectiles_player.projectile_queue.size(); i++) {
        auto e = entity_manager.create();
        projectiles_player.create(e, projectiles_player.projectile_queue[i].position, projectiles_player.projectile_queue[i].velocity);
    }
    projectiles_player.projectile_queue.clear();

    for(size_t i = 0; i < projectiles_target.projectile_queue.size(); i++) {
        auto e = entity_manager.create();
        projectiles_target.create(e, projectiles_target.projectile_queue[i].position, projectiles_target.projectile_queue[i].velocity);
    }
    projectiles_target.projectile_queue.clear();
}

void GameArea::spawn_effects() {
    for(size_t i = 0; i < effects.effect_queue.size(); i++) {
        auto e = entity_manager.create();
        effects.create(e, effects.effect_queue[i].position, effects.effect_queue[i].velocity, effects.effect_queue[i].sprite, effects.effect_queue[i].effect);
    }
    effects.effect_queue.clear();
}

void GameArea::spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent) {
    auto spr = SpriteComponent("shooter", "bullet_1.png");
    spr.layer = 2;
    auto effect = EffectData(2 * Time::delta_time_fixed);
    effect.follow = parent;
    effect.local_position = local_position;
    effect.has_target = true;
    effects.effect_queue.push_back({ p, Velocity(), spr, effect });
}

void GameArea::spawn_explosion(Vector2 position, float offset_x, float offset_y) {
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
    effects.effect_queue.push_back({ { blast_position }, Velocity(), spr, effect });
}