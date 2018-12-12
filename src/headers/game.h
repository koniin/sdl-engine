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

// Dont know if we need this to be inherited
struct Target_Test : ECS::EntityDataDynamic {};
struct Player_Test : ECS::EntityDataDynamic {};
struct Projectile_Test : ECS::EntityDataDynamic {};

// ugly shit
// should be in framework.cpp,
THIS DOES NOT NEED TO BE STATIC, CAN JUST BE COUNTER IN THE DYNAMICCONTAINER I THINK
size_t ECS::EntityDataDynamic::TypeID::counter = 0;

ECS::EntityManager em;
Player_Test players;
Target_Test targets;
Projectile_Test projectiles;

void spawn_player(int p) {

}

void spawn_target(int p) {
	auto ent = em.create();
	targets.add_entity(ent);
	auto handle = targets.get_handle(ent);
	Position_T &pos = targets.get<Position_T>(handle);
	pos.x = p;

	auto handle2 = targets.get_handle(ent);
	const Position_T pos2 = targets.get<Position_T>(handle);
	Engine::logn("%d", pos2.x);
}

inline void test_dynamic() {
	Engine::logn(" ---  ECS TEST  --- ");
	
	players.allocate_entities<Position_T, Velocity_T, Health_T>(23);
	targets.allocate_entities<Position_T, Velocity_T, Health_T, AI_T>(23);
	projectiles.allocate_entities<Position_T, Velocity_T, Damage_T>(23);

	spawn_player(555);
	spawn_target(33);
	

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