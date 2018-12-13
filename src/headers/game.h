#ifndef GAME_H
#define GAME_H

#include "engine.h"
#include "renderer.h"
#include "rooms.h"
#include <chrono>


/* 
	How it works 
	- allocate memory for everything on application start
	
	- load all resources used in the application  ( during a loading screen ? )
		- we could also init sound at this screen somehow so init is faster and no white screen
*/ 

#include "level\framework.h"

struct Position_T { int x; };
struct Velocity_T { int v; };
struct Health_T { int h; };
struct AI_T { int a; };
struct Damage_T { int d; };

// Mål
// 1. Ersätta Player, Target, Projectiles etc i entities.h med ett enkelt dynamiskt API
//	- auto c = create_container<Position, Velocity>();
// 2. Enkelt skapa entity? 
//	- create_entity(c);
// 2. Enkelt få alla som har en viss uppsättning komponenter
//  - container_iterator = get_containers<Position, Velocity>();
//  - for(int i = 0; i < container_iterator.length; i++) {
// 			container_iterator.get<Position>(i).value = 
//    }


// Fix later
size_t ECS::ComponentID::counter = 0;

ECS::ArchetypeManager em;
ECS::ArcheType players;
ECS::ArcheType targets;
ECS::ArcheType projectiles;

void spawn_player(int p) {
	auto ent = em.create_entity(players);
	auto handle = em.get_handle(players, ent);

	Position_T pos = { 66 };
	em.set_component(players, handle, pos);
	Position_T &pos_get = em.get_component<Position_T>(players, handle);
	Engine::logn("pos_get: %d", pos_get.x);
	// players->add_entity(ent);

	// auto handle = players->get_handle(ent);
	// Position_T &pos = players->get<Position_T>(handle);
	// pos.x = p;

	// auto handle2 = players->get_handle(ent);
	// const Position_T pos2 = players->get<Position_T>(handle);
	// Engine::logn("%d", pos2.x);
}

void spawn_target(int p) {
	auto ent = em.create_entity(targets);
	auto handle = em.get_handle(targets, ent);
	Position_T pos = { 789 };
	em.set_component(targets, handle, pos);

	em.remove_entity(targets, ent);
}
// 	auto ent = em.create();
// 	targets->add_entity(ent);
// 	auto handle = targets->get_handle(ent);
// 	targets->set<Position_T>(handle, { p });
	
// 	auto handle2 = targets->get_handle(ent);
// 	const Position_T pos2 = targets->get<Position_T>(handle);
// 	Engine::logn("%d", pos2.x);

// 	auto h3 = players->get_handle(ent);
// 	if(targets->is_valid_handle(h3)) {
// 		Engine::logn("This is not what you want");
// 	} else {
// 		Engine::logn("handle is invalid = OK");
// 	}
// }

// void remove_target() {
// 	Engine::logn("-- Remove entity test");
// 	Engine::logn("target count: %d", targets->length);
// 	auto entity = targets->entity[0];
// 	targets->remove(entity);

// 	auto h = targets->get_handle(entity);
// 	if(targets->is_valid_handle(h)) {
// 		Engine::logn("This is wrong");
// 	} else {
// 		Engine::logn("Handle is invalid - good shit");
// 	}
// }

inline void test_dynamic() {
	Engine::logn(" ---  ECS TEST  --- ");
	
	Engine::logn(" init players ");
	players = em.create_archetype<Position_T, Velocity_T, Health_T>(23);
	//players.allocate_entities<Position_T, Velocity_T, Health_T>(23);
	Engine::logn(" init targets ");
	targets = em.create_archetype<Position_T, Velocity_T, Health_T, AI_T>(23);
	Engine::logn(" init projectiles ");
	projectiles = em.create_archetype<Position_T, Velocity_T, Damage_T>(23);

	spawn_player(555);
	spawn_target(33);
	// remove_target();
	// spawn_target(77);

	auto ci = em.get_containers<Velocity_T, Health_T, Position_T>();
	for(auto c : ci.containers) {
		if(c->length == 0) {
			// Just for test
			continue;
		}

		Engine::logn("length: %d", c->length);
		for(int i = 0; i < c->length; i++) {
			Position_T &p = c->index<Position_T>(i);
			Engine::logn("x: %d", p.x);
		}
		
		auto a = c->entity[0];
		auto h = c->get_handle(a);
		if(c->is_valid_handle(h)) {
			auto pos2 = c->get<Position_T>(h);
			Engine::logn("x2: %d", pos2.x);
		}
	}

	Engine::logn(" ---  ECS TEST END  --- ");
}

inline void game_load() {
	test_dynamic();

	// Allocate memory and load resources
	Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
    FrameLog::enable_at(5, 5);
	
	room_load_all();

	room_goto(Rooms::MainMenu);
}

inline void game_update() {
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	// bullet_update();
	// collision_test_update();
	// // asteroids_update();
	// // tile_collisions_update();
	
	// shooter_update();
	room_update();
	
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
	std::string frame_duration_mu = "update time mu: " + std::to_string(duration);
	std::string frame_duration_ms = "update time ms: " + std::to_string(duration_ms);
	FrameLog::log(frame_duration_mu);
	FrameLog::log(frame_duration_ms);
    // std::cout << "\n" << duration << " MICRO seconds";

	
	// update_particle_editor();
}

inline void game_render() {
	renderer_clear();
	room_render();
	renderer_draw_render_target_camera();
	room_render_ui();
	renderer_flip();
}

inline void game_unload() {
	room_unload_all();
}

#endif