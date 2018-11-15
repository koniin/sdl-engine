#ifndef GAME_H
#define GAME_H

#include "engine.h"
#include "renderer.h"
// #include "asteroids_ecs.h"
// #include "tile_collisions.h"
// #include "asteroids.h"
// #include "bullet_physics.h"
#include "shooter_game.h"
#include "menu.h"
// #include "particle_editor.h"
// #include "collision_tests.h"
#include <chrono>

// static TileMap the_map;
// static SpriteSheet the_sheet;


/* 
	How it works 
	- allocate memory for everything on application start
	- load all resources used in the application
	- shooter_game has a method to setup entities and "level" so you call that when starting a level
		- it will just clear everything current (not freeing any memory) and load the new data into it's caches
	
	- all menu screens also have methods to allocate their data and render methods

	- rendering states is just a matter of calling methods
	

*/ 



inline void game_load() {
	// Allocate memory and load resources
	Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
    FrameLog::enable_at(5, 5);
	
	shooter_load();
	menu_load();

}

inline void game_update() {
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	// bullet_update();
	// collision_test_update();
	// // asteroids_update();
	// // tile_collisions_update();
	
	// shooter_update();

	menu_update();

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
	//draw_sprite_centered(Resources::sprite_get("bkg"), gw / 2, gh / 2);
	/*
	int x_start = 20, y_start = 20;
	draw_tilemap_ortho(the_map, the_sheet, x_start, y_start);
	*/
	//draw_spritesheet_name_centered_rotated(the_sheet, "middle", enemy.x, enemy.y, enemy.angle);

	
	// // asteroids_render();
	// // tile_collisions_render();
	// // bullet_render();
	// // collision_test_render();
	
	renderer_clear();
	
	//shooter_render();
	menu_render();

	renderer_draw_render_target_camera();
	
	//shooter_render_ui();
	menu_render_ui();
	
	renderer_flip();

	
	// renderer_clear();
	// render_particle_editor();
	// renderer_draw_render_target_camera();
	// renderer_flip();
}

inline void game_unload() {
	shooter_unload();
}


/*
inline void update_following_enemy(Ship &s, Ship &target) {
	float distance = Math::distance_f(s.x, s.y, target.x, target.y);
	if(distance > 30) {
		auto rotation = Math::rads_between_f(s.x, s.y, target.x, target.y);
		s.angle = rotation * Math::RAD_TO_DEGREE;
		
		float direction_x = cos(rotation);
		float direction_y = sin(rotation);
		s.velocity_x += direction_x * config.acceleration;
		s.velocity_y += direction_y * config.acceleration;
		
		s.x += s.velocity_x;
		s.y += s.velocity_y;
	} else {
		s.velocity_x = 0;
		s.velocity_y = 0;
	}
}*/

#endif