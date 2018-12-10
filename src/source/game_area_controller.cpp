#include "level\game_area_controller.h"

void GameAreaController::spawn_player(const Vector2 &position) {
    auto e = game_area->entity_manager.create();
    game_area->players.create(e, position);
}

void GameAreaController::spawn_target(const Vector2 &position, const Enemy &e) {
    auto entity = game_area->entity_manager.create();
    game_area->targets.create(entity, position, e);
}

void GameAreaController::spawn_projectiles() {
    for(size_t i = 0; i < player_projectile_queue.size(); i++) {
        auto e = game_area->entity_manager.create();
        SpriteComponent s = SpriteComponent("shooter", "bullet_2");
        ProjectileSpawn &p = player_projectile_queue[i];
        if(p.line) {
            s.line = true;
            s.position = Vector2(p.line_rect.x, p.line_rect.y);
            s.w = p.line_rect.w;
            s.h = p.line_rect.h;
            s.sprite_name = "lazer";
            s.rotation = p.angle;
            s.radius = p.line_rect.h / 2;
        }
        game_area->projectiles_player.create(e, p, s);
    }
    player_projectile_queue.clear();

    for(size_t i = 0; i < target_projectile_queue.size(); i++) {
        auto e = game_area->entity_manager.create();
        SpriteComponent s = SpriteComponent("shooter", "bullet_2");
        game_area->projectiles_target.create(e, target_projectile_queue[i], s);
    }
    target_projectile_queue.clear();
}

void GameAreaController::spawn_effects() {
    for(size_t i = 0; i < game_area->effects.effect_queue.size(); i++) {
        auto e = game_area->entity_manager.create();
        game_area->effects.create(e, 
            game_area->effects.effect_queue[i].position, 
            game_area->effects.effect_queue[i].velocity, 
            game_area->effects.effect_queue[i].sprite, 
            game_area->effects.effect_queue[i].effect);
    }
    game_area->effects.effect_queue.clear();
}

void GameAreaController::spawn_muzzle_flash_effect(Vector2 p, Vector2 local_position, ECS::Entity parent) {
    auto spr = SpriteComponent("shooter", "bullet_1");
    spr.layer = 2;
    auto effect = EffectData(2 * Time::delta_time_fixed);
    effect.follow = parent;
    effect.local_position = local_position;
    effect.has_target = true;
    game_area->effects.effect_queue.push_back({ { p }, Velocity(), spr, effect });
}

void GameAreaController::spawn_explosion_effect(Vector2 position, float offset_x, float offset_y) {
    auto spr = SpriteComponent("shooter", "explosion_1");
    spr.layer = 0;
    auto effect = EffectData(4 * Time::delta_time_fixed);
    effect.modifier_enabled = true;
    effect.modifier_data_s = "explosion_2";
    effect.modifier_time = 2 * Time::delta_time_fixed;
    effect.modifier = sprite_effect;
    Vector2 blast_position = position;
    blast_position.x += RNG::range_f(-offset_x, offset_x);
    blast_position.y += RNG::range_f(-offset_y, offset_y);
    game_area->effects.effect_queue.push_back({ { blast_position }, Velocity(), spr, effect });

    // Particles
    game_area->explosion_emitter.position = position;
    Particles::emit(game_area->particles, game_area->explosion_emitter);
}

void GameAreaController::spawn_hit_effect(Vector2 position, float angle) {
    game_area->hit_emitter.position = position;
    game_area->hit_emitter.angle_min = angle - 10.0f;
    game_area->hit_emitter.angle_max = angle + 10.0f;
    Particles::emit(game_area->particles, game_area->hit_emitter);
}