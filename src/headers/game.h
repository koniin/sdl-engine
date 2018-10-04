#ifndef GAME_H
#define GAME_H

#include "engine.h"
#include "renderer.h"
#include "asteroids_ecs.h"
// #include "tile_collisions.h"
// #include "asteroids.h"
// #include "ecs_bitsquid.h"
// #include "ecs.h"
// #include "ecs_one.h"
// #include "ecs_static.h"

// ECS UNITY MOTHERLOAD => https://forum.unity.com/threads/ecs-memory-layout.532028/
/*
	https://github.com/Unity-Technologies/EntityComponentSystemSamples/blob/master/Documentation/content/ecs_in_detail.md
	https://eli.thegreenplace.net/2014/variadic-templates-in-c/
	https://medium.com/@savas
	http://lazyfoo.net/tutorials/SDL/39_tiling/index.php
	https://github.com/Unity-Technologies/EntityComponentSystemSamples/blob/master/Documentation/content/two_stick_shooter.md
	https://github.com/Unity-Technologies/EntityComponentSystemSamples/blob/132f511a0f36d2bb422fc807cb3a808ea18d7df5/Samples/Assets/TwoStickShooter/Pure/Scripts/ComponentTypes.cs
	https://blog.therocode.net/2018/08/simplest-entity-component-system
	https://github.com/eigenbom/game-example/blob/464498d569dc4dab55e621321dc260a9773c29b5/src/mobsystem.cpp
	
*/

// static TileMap the_map;
// static SpriteSheet the_sheet;

inline void game_load() {
	asteroids_load();

	// Resources::sprite_load("bkg", "bkg.png");
	// Tiling::tilemap_load("tilemap.txt", the_map);
	// Resources::sprite_sheet_load("shooter.data", the_sheet);
	
	// tile_collisions_load();
}

inline void game_update() {
	asteroids_update();
	// tile_collisions_update();
}

inline void game_render() {
	//draw_sprite_centered(Resources::sprite_get("bkg"), gw / 2, gh / 2);
	/*
	int x_start = 20, y_start = 20;
	draw_tilemap_ortho(the_map, the_sheet, x_start, y_start);
	*/
	//draw_spritesheet_name_centered_rotated(the_sheet, "middle", enemy.x, enemy.y, enemy.angle);

	renderer_clear();
	
	asteroids_render();
	//tile_collisions_render();

	renderer_draw_render_target();
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