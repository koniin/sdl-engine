
#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

#include "engine.h"
#include "renderer.h"
#include "entities.h"
#include "systems.h"
#include "particles.h"
#include "emitter_config.h"

static const size_t RENDER_BUFFER_MAX = 256;

struct LevelConfig {

};

struct Level {
    Rectangle world_bounds;

    ECS::EntityManager entity_manager;
    Player players;
    Projectile projectiles_player;
    Projectile projectiles_target;
    Target targets;
    Effect effects;
    
    CollisionPairs collisions;

    Particles::ParticleContainer particles;
    Particles::Emitter explosion_emitter;
    Particles::Emitter hit_emitter;
    Particles::Emitter exhaust_emitter;
    Particles::Emitter smoke_emitter;

    Level() {
        particles = Particles::make(4096);
        players.allocate(1);
        projectiles_player.allocate(128);
        projectiles_target.allocate(256);
        targets.allocate(128);
        effects.allocate(128);
        collisions.allocate(128);

        emitters_configure(this);
    }

    void remove_deleted_entities() {
        system_remove_deleted(players);
        system_remove_deleted(projectiles_player);
        system_remove_deleted(projectiles_target);
        system_remove_deleted(targets);
        system_remove_deleted(effects);
    }

    void spawn_projectiles() {
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

    void spawn_effects() {
        for(size_t i = 0; i < effects.effect_queue.size(); i++) {
            auto e = entity_manager.create();
            effects.create(e, effects.effect_queue[i].position, effects.effect_queue[i].velocity, effects.effect_queue[i].sprite, effects.effect_queue[i].effect);
        }
        effects.effect_queue.clear();
    }

    void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent) {
        auto spr = SpriteComponent("shooter", "bullet_1.png");
        spr.layer = 2;
        auto effect = EffectData(2 * Time::delta_time_fixed);
        effect.follow = parent;
        effect.local_position = local_position;
        effect.has_target = true;
        effects.effect_queue.push_back({ p, Velocity(), spr, effect });
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
        effects.effect_queue.push_back({ { blast_position }, Velocity(), spr, effect });
    }

	// WorldBounds bounds;
	// std::vector<AvoidThis> avoidThis;
	// std::vector<RegularObject> regularObject;

	// Game(const WorldBoundsComponent& bounds)
	// 	: bounds(bounds)
	// {
	// 	// create regular objects that move
	// 	regularObject.reserve(kObjectCount);
	// 	for (auto i = 0; i < kObjectCount; ++i)
	// 		regularObject.emplace_back(bounds);

	// 	// create objects that should be avoided
	// 	avoidThis.reserve(kAvoidCount);
	// 	for (auto i = 0; i < kAvoidCount; ++i)
	// 		avoidThis.emplace_back(bounds);
	// }
	// void Clear()
	// {
	// 	avoidThis.clear();
	// 	regularObject.clear();
	// }
};

void level_load();
void level_init();
void level_unload();
void level_update();
void level_render();
void level_render_ui();

#endif