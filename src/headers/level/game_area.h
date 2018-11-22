#ifndef GAME_AREA_H
#define GAME_AREA_H

#include "engine.h"
#include "renderer.h"
#include "entities.h"
#include "particles.h"
#include "emitter_config.h"
#include "game_events.h"

struct GameArea {
    Rectangle world_bounds;
    SDL_Color background_color;

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
        
    void clear() {
        players.clear();
        projectiles_player.clear();
        projectiles_target.clear();
        targets.clear();
        effects.clear();
        particles.length = 0;
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

#endif