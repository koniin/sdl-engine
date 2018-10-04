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

template<typename T>
struct ForwardIndexer {
	unsigned length = 0;
	
	struct Cache {
		T *cache_ptr;
		unsigned cached_begin = 0;
		unsigned cached_end = 0;
		int data_n = 0;
		int data_ptr = 0;
		T *data[1024];
		size_t datasizes[1024];
	} cache;

	template <size_t t_n> 
	void add(T (&data)[t_n]) {
		if(cache.cached_end == 0) {
			cache.cached_begin = 0;
			cache.data_ptr = 0;
			cache.cached_end = t_n;
		}
		cache.datasizes[cache.data_n] = t_n;
		cache.data[cache.data_n++] = data;
		cache.cache_ptr = cache.data[cache.data_ptr];
		length += t_n;
	}

	T &index(unsigned i) {
		ASSERT_WITH_MSG(i < length, "index out of bounds");
		
		if(i >= cache.cached_end) {
			update_cache(i);
		}
		return static_cast<T&>(*(cache.cache_ptr + i));
	}

	void update_cache(unsigned i) {
		auto size = cache.datasizes[cache.data_ptr];
		cache.cached_begin += size;
		cache.cached_end += size;
		cache.cache_ptr = (cache.data[++cache.data_ptr] - cache.cached_begin);
	}
};

struct PositionTest {
	float x;
	float y;
};

inline void game_load() {
	asteroids_load();

	// int ar1[5] = {1, 2, 3, 4, 5};
	// int ar2[5] = {6, 7, 8, 9, 10};
	// int* arp[2] = { ar1, ar2 };
	
	// int *ar1ptr = ar1;
	
	// Engine::logn("%d", ar1ptr[2]);
	// Engine::logn("%d", *(ar1ptr + 2));
	
	PositionTest p1[2] = { { 1.0f, 1.0f }, { 2.0f, 2.0f } };
	PositionTest p2[2] = { { 3.0f, 3.0f }, { 4.0f, 4.0f } };

	for(unsigned i = 0; i < 2; ++i) {
		Engine::logn("i: %d , position: %f.0, %f.0", i, p1[i].x, p1[i].y);

	}
	
	// f.add(p2);
	/*
	Apa<Kuken> a;

	Engine::logn("sizeof 1: %d", sizeof(Kuken));
	Engine::logn("sizeof 2: %d", sizeof(a.data));
*/
	ForwardIndexer<PositionTest> f;
	f.add(p1);
	f.add(p2);
	Engine::logn("items: %d", f.length);
	for(unsigned i = 0; i < f.length; ++i) {
		PositionTest &p = f.index(i);
		Engine::logn("i: %d , position: %f.0, %f.0", i, p.x, p.y);
		p.x = 100.0f + (float)i;
	}

	f = ForwardIndexer<PositionTest>();
	f.add(p1);
	f.add(p2);
	Engine::logn("after");
	for(unsigned i = 0; i < f.length; ++i) {
		PositionTest &p = f.index(i);
		Engine::logn("i: %d , position: %f.0, %f.0", i, p.x, p.y);
	}


	// AtomicSafetyHandle.CheckReadAndThrow(this.m_Safety);
	// 		if (index >= this.m_Length)
	// 		{
	// 			this.FailOutOfRangeError(index);
	// 		}
	// 		if (index < this.m_Cache.CachedBeginIndex || index >= this.m_Cache.CachedEndIndex)
	// 		{
	// 			this.m_Iterator.MoveToEntityIndexAndUpdateCache(index, out this.m_Cache, false);
	// 		}
	// 		return UnsafeUtility.ReadArrayElement<Entity>(this.m_Cache.CachedPtr, index);
	

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