#ifndef GAME_H
#define GAME_H

#include "engine.h"
#include "renderer.h"
// #include "asteroids_ecs.h"
// #include "tile_collisions.h"
// #include "asteroids.h"
// #include "bullet_physics.h"
#include "shooter_game.h"
// #include "collision_tests.h"
#include <chrono>

// static TileMap the_map;
// static SpriteSheet the_sheet;

// struct PositionTest {
// 	float x;
// 	float y;
// };

inline void game_load() {
	// asteroids_load();
	// bullet_load();
	load_shooter();
	// collision_test_load();

	// Resources::sprite_load("bkg", "bkg.png");
	// Tiling::tilemap_load("tilemap.txt", the_map);
	// Resources::sprite_sheet_load("shooter.data", the_sheet);
	// tile_collisions_load();
}

inline void game_update() {
	// bullet_update();
	

	// std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	
	// collision_test_update();
	update_shooter();

	// // asteroids_update();
	// // tile_collisions_update();
	// std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    // std::cout << "\n" << duration << " MICRO seconds";
}

inline void game_render() {
	//draw_sprite_centered(Resources::sprite_get("bkg"), gw / 2, gh / 2);
	/*
	int x_start = 20, y_start = 20;
	draw_tilemap_ortho(the_map, the_sheet, x_start, y_start);
	*/
	//draw_spritesheet_name_centered_rotated(the_sheet, "middle", enemy.x, enemy.y, enemy.angle);

	renderer_clear();
	
	// asteroids_render();
	// tile_collisions_render();
	// bullet_render();

	render_shooter();
	// collision_test_render();

	renderer_draw_render_target_camera();

	render_shooter_ui();

	renderer_flip();
}

inline void game_unload() {

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