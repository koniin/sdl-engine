#ifndef GAME_AREA_H
#define GAME_AREA_H

#include "engine.h"
#include "renderer.h"
#include "entities.h"
#include "particles.h"
#include "emitter_config.h"

struct GameArea {
    Rectangle world_bounds;

    ECS::EntityManager entity_manager;
    Player players;
    Projectile projectiles_player;
    Projectile projectiles_target;
    Target targets;
    Effect effects;
    
    Particles::ParticleContainer particles;
    Particles::Emitter explosion_emitter;
    Particles::Emitter hit_emitter;
    Particles::Emitter exhaust_emitter;
    Particles::Emitter smoke_emitter;

    GameArea() {
        particles = Particles::make(4096);
        players.allocate(1);
        projectiles_player.allocate(128);
        projectiles_target.allocate(256);
        targets.allocate(128);
        effects.allocate(128);

        emitters_configure(this);
    }

    void load(const Rectangle &bounds);
    void spawn_player(Vector2 position);
    void spawn_target(Vector2 position);
    void remove_deleted_entities();
    void spawn_projectiles();
    void spawn_effects();
    void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent);
    void spawn_explosion(Vector2 position, float offset_x, float offset_y);

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

#endif