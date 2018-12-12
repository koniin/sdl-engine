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

struct Position_T { int x; };
struct Velocity_T { int v; };

#include "level\framework.h"
struct Target_Test : ECS::EntityDataDynamic {};

// ugly shit
// should be in framework.cpp
size_t ECS::EntityDataDynamic::TypeID::counter = 0;

inline void test_dynamic() {
	ECS::EntityManager em;
	Target_Test tt;
	tt.allocate_entities<Position_T, Velocity_T>(23);
	auto ent = em.create();
	tt.add_entity(ent);
	auto handle = tt.get_handle(ent);
	Position_T &pos = tt.get<Position_T>(handle);
	pos.x = 23;

	auto handle2 = tt.get_handle(ent);
	const Position_T pos2 = tt.get<Position_T>(handle);

	Engine::logn("%d", pos2.x);
	// tt.containers[0]
	//tt.init(23);
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