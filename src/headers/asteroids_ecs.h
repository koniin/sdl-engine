
// TODO:
// 1. Make a fix for the reset function on ComponentArray
//      either a new collection that is just forward or do some magic on the
//      update_cache, perhaps check if its one more etc
// 2. Fix faction in bullet firing in player move system
// 3. Fill indexer by archetype 
// 4. 
// 5. 


#ifndef ASTEROIDS_ECS_H
#define ASTEROIDS_ECS_H

#include "asteroids_ecs_tech.h"

struct AsteroidsConfig {
	float rotation_speed = 5.0f; 
	float acceleration = 0.2f;
	float brake_speed = -0.05f;
	float drag = 0.02f;
	float fire_cooldown = 0.25f; // s
	float player_bullet_speed = 5;
	float player_bullet_size = 1;
	int player_faction_1 = 0;
	int player_faction_2 = 1;
	int enemy_faction = 2;
	float player_death_inactive_time = 1.0f;
	float player_shield_time = 2.0f;
	float player_shield_inactive_time = 6.0f;
	int asteroid_count_increase_per_level = 2;
	SDL_Color asteroid_color = { 240, 240, 240, 255 };
    SDL_Color bullet_color = { 255, 0, 0, 255 };
} config;

struct GameState {
	bool inactive = false;
	float inactive_timer = 0.0f;
	float pause_time = 2.0f;
	int level = 0;
	SDL_Color text_color = { 220, 220, 220, 255 };
    int player_score_1 = 0;
} game_state;

struct InputMapping {
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
	SDL_Scancode fire;
	SDL_Scancode shield;
};

InputMapping input_maps[2] = {
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_SPACE, SDL_SCANCODE_LSHIFT },
	{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RSHIFT }
};

struct PlayerInput {
	// Input
	float move_x = 0;
	float move_y = 0;
	float fire_x = 0;
	float fire_y = 0;
	float fire_cooldown = 0;
	bool shield = false;
};

struct Position {
    Vector2 value;
};

struct Velocity {
    Vector2 value;
};

struct Direction {
    float angle = 0;
};

struct SizeComponent { 
    float radius = 0;
};

struct Faction {
    int faction = 0;
};

struct Health {
	int value = 0;
};

struct Shield {
	float active_timer = 0;
	float inactive_timer = 0;
	bool is_active() const {
		return active_timer > 0.0f;
	}
};

// Rendering
struct ColorComponent {
    SDL_Color color;
};

// Purely for tagging
struct MoveForwardComponent {};
struct WrapAroundMovement {};

// Events

struct ShotSpawnData {
	Vector2 position;
	Vector2 direction;
	int faction;
};

struct DestroyEntityData {
    Entity entity;
};

World *world;
EntityManager *entity_manager;
SpriteSheet the_sheet;
EntityArchetype player_archetype;
EntityArchetype bullet_archetype;
EntityArchetype asteroid_archetype;

// Application specific event queue wrapper
EventQueue event_queue;
template<typename T>
void queue_event(T *d) {
    event_queue.queue_evt(d);
}

void spawn_player() {
    auto e = entity_manager->create_entity(player_archetype);
    
    // Set some component data
    entity_manager->set_component<PlayerInput>(e, { 0, 0, 0, 0, 0, false });
    entity_manager->set_component<Direction>(e, { 0 });
    Vector2 pos = Vector2((float)gw / 2, (float)gh / 2);
    Vector2 vel = Vector2::Zero;
    Position pc = { pos };
    Velocity vc = { vel };
    entity_manager->set_component(e, pc);
    entity_manager->set_component(e, vc);
    entity_manager->set_component<Health>(e, { 3 });
    entity_manager->set_component<SizeComponent>(e, { 7 });

    Engine::logn("Position: %f", pc.value.x);
    Engine::logn("Entity: %d", e.id);
}

void spawn_bullet(Vector2 position, Vector2 direction, int faction) {
    auto bullet = entity_manager->create_entity(bullet_archetype);
    Engine::logn("new bullet, x: %f, y: %f", position.x, position.y);

    // SOMETHING WITH CREATIING ENTITIES AFTER THEY HAVE BEEN DESTROYED IS WHACK

    entity_manager->set_component<Position>(bullet, { position });
    entity_manager->set_component<Faction>(bullet, { faction });
    entity_manager->set_component<ColorComponent>(bullet, { config.bullet_color });
    if(faction == config.player_faction_1 || faction == config.player_faction_2) {
        entity_manager->set_component<Velocity>(bullet, { 
            Vector2(direction.x * config.player_bullet_speed,
                direction.y * config.player_bullet_speed)
        });
        entity_manager->set_component<SizeComponent>(bullet, { config.player_bullet_size });
    } else {
        ASSERT_WITH_MSG(0, "FACTION NOT IMPLEMENTED");
    }
}

void handle_events() {
    for(auto &e : event_queue.events) {
		if(e.is<ShotSpawnData>()) {
            ShotSpawnData *d = e.get<ShotSpawnData>();
			spawn_bullet(d->position, d->direction, d->faction);
        } else if(e.is<DestroyEntityData>()) {
            DestroyEntityData *d = e.get<DestroyEntityData>();
            entity_manager->destroy_entity(d->entity);
        }
        // e.destroy(); <- only needed if manually emptying the queue
    }
    
    event_queue.clear();
}

float asteroid_radius(const int size) {
    if(size == 1)
		return 16.0f;
	else if(size == 2) 
		return 8.0f;
	else 
		return 4.0f;
}

void spawn_asteroid(Position position, Velocity velocity, int size) {
    Entity asteroid = entity_manager->create_entity(asteroid_archetype);
    entity_manager->set_component(asteroid, position);
    entity_manager->set_component(asteroid, velocity);
    entity_manager->set_component<SizeComponent>(asteroid, { asteroid_radius(size) });
    entity_manager->set_component<ColorComponent>(asteroid, { config.asteroid_color });

    Engine::logn("spawning asteroid: x: %f, y: %f, size: %d, radius: %f",
        position.value.x, position.value.y, size, asteroid_radius(size));
}

void spawn_asteroid_wave() {
	for(int i = 0; i < game_state.level + config.asteroid_count_increase_per_level; ++i) {
		Position position = { Vector2(RNG::range_f(0, (float)gw), RNG::range_f(0, (float)gh)) };
        Velocity velocity = { Vector2(RNG::range_f(0, 100) / 100.0f - 0.5f, RNG::range_f(0, 100) / 100.0f - 0.5f) };
		int new_asteroid_size = 1;
		spawn_asteroid(position, velocity, new_asteroid_size);
	}
}

void system_asteroid_spawn() {
    size_t asteroids = entity_manager->archetype_count(asteroid_archetype);
	if(asteroids == 0) {
		game_state.level++;
		spawn_asteroid_wave();
	}
}

void system_shield() {
    ComponentArray<PlayerInput> player_input;
    ComponentArray<Shield> shield;
    unsigned length;
    world->fill_by_arguments(length, player_input, shield);
    for(unsigned i = 0; i < length; ++i) {
		Shield &s = shield[i];
		s.active_timer = Math::max_f(0.0f, s.active_timer - Time::deltaTime);
		s.inactive_timer = Math::max_f(0.0f, s.inactive_timer - Time::deltaTime);
		PlayerInput &pi = player_input[i];
		if(pi.shield && s.inactive_timer <= 0.0f) {
			s.active_timer = config.player_shield_time;
			s.inactive_timer = config.player_shield_time + config.player_shield_inactive_time;
		}
	}
}

inline void system_player_input() {
    ComponentArray<PlayerInput> fp;
    world->fill<PlayerInput>(fp);

    //Engine::logn("PlayerInput: %d", fp.length);
    for(unsigned i = 0; i < fp.length; ++i) {
		PlayerInput &pi = fp.index(i);
		//Engine::logn("i: %d , input: %f.0, %f.0", i, pi.move_x, pi.move_y);

        pi.move_x = 0;
        pi.move_y = 0;
        pi.fire_x = 0;
        pi.fire_y = 0;
        pi.shield = false;
        
	    InputMapping key_map = input_maps[0];
        if(Input::key_down(key_map.up)) {
            pi.move_y = 1;
        } else if(Input::key_down(key_map.down)) {
            pi.move_y = -1;
        } 
        
        if(Input::key_down(key_map.left)) {
            pi.move_x = -1;
        } else if(Input::key_down(key_map.right)) {
            pi.move_x = 1;
        }

        pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::deltaTime);
        if(Input::key_down(key_map.fire)) {
            pi.fire_x = pi.fire_y = 1;
        }

        if(Input::key_down(key_map.shield)) {
            pi.shield = true;
        }
	}
}

inline void system_player_movement() {
    ComponentArray<PlayerInput> fpi;
    ComponentArray<Position> fp;
    ComponentArray<Velocity> fv;
    ComponentArray<Direction> fd;
    unsigned length;
    world->fill_by_arguments(length, fpi, fp, fv, fd);
    
    // Engine::logn("PlayerInput: %d", fp.length);
    for(unsigned i = 0; i < length; ++i) {
		PlayerInput &pi = fpi.index(i);
        Position &position = fp.index(i);
        Velocity &velocity = fv.index(i);
        Direction &direction = fd.index(i);

        // Update rotation based on rotational speed
        // for other objects than player input once
        direction.angle += pi.move_x * config.rotation_speed;
        float rotation = direction.angle / Math::RAD_TO_DEGREE;

        float direction_x = cos(rotation);
        float direction_y = sin(rotation);
        velocity.value.x += direction_x * pi.move_y * config.acceleration;
        velocity.value.y += direction_y * pi.move_y * config.acceleration;
        
        position.value.x +=  velocity.value.x;
        position.value.y +=  velocity.value.y;

        // Use Stokes' law to apply drag to the object
        velocity.value.x = velocity.value.x - velocity.value.x * config.drag;
        velocity.value.y = velocity.value.y - velocity.value.y * config.drag;
        
        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            ShotSpawnData *d = new ShotSpawnData;
            d->position = position.value;
            d->direction.x = direction_x;
            d->direction.y = direction_y;            
            // TODO: FIX FACTION
            d->faction = config.player_faction_1;
            queue_event(d);
            
            pi.fire_cooldown = config.fire_cooldown;
        }
    }
}

inline void system_forward_movement() {    
    ComponentArray<Velocity> velocity;
    ComponentArray<Position> position;
    unsigned length;
    world->fill_by_types<Velocity, Position, MoveForwardComponent>(length, velocity, position);
    
    for(unsigned i = 0; i < length; ++i) {
        Velocity &v = velocity[i];
        Position &p = position[i];
        p.value += v.value;
    }
}

inline void system_keep_in_bounds() {
    ComponentArray<Position> position;
    world->fill<Position, WrapAroundMovement>(position);
    for(unsigned i = 0; i < position.length; ++i) {
        Position &p = position[i];
        if(p.value.x < 0) p.value.x = (float)gw;
        if(p.value.x > gw) p.value.x = 0.0f;
        if(p.value.y < 0) p.value.y = (float)gh;
        if(p.value.y > gh) p.value.y = 0.0f;
    }
}

void system_collisions() {
    struct CollisionGroup : EntityComponentData<Position, SizeComponent> {
        ComponentArray<Position> position;
        ComponentArray<SizeComponent> collision_data;
    };
    CollisionGroup a, b;
    world->fill_entity_data(a, a.entities, a.position, a.collision_data);
    world->fill_entity_data(b, b.entities, b.position, b.collision_data);

    // ComponentArray<Entity> entities;
    // unsigned length;
    // world->fill_by_types<Position, SizeComponent>(length, position, collision_data);
    // world->fill_entities<Position, SizeComponent>(entities);
    //Log::add_message("entities that will collide: %d == %d", length, entities.length);

    struct CollisionPair {
        Entity a;
        Entity b;
    };
    std::vector<CollisionPair> collisions;
    for(unsigned i = 0; i < a.length; ++i) {
        const Vector2 first_position = a.position[i].value;
        const float first_radius = a.collision_data[i].radius;
        const Entity first_entity = a.entities[i];
        for(unsigned j = 0; j < b.length; ++j) {
            const Vector2 second_position = b.position[j].value;
            const float second_radius = b.collision_data[j].radius;
            const Entity second_entity = b.entities[j];
            if(i != j 
                && Math::intersect_circles(first_position.x, first_position.y, first_radius, 
                    second_position.x, second_position.y, second_radius)) {
                collisions.push_back({ 
                    first_entity,
                    second_entity
                });
                //queue_event({ Event::ShipHit, new ShipHitData { ships[si].faction }});
			}
        }
        b.position.reset();
        b.collision_data.reset();
        b.entities.reset();
    }

    // Maybe do this somewhere else?

    for(auto &c : collisions) {
        Engine::logn("collision = %d <> %d", c.a.id, c.b.id);
        
        // player => asteroid
        // instead of wraparound it should be DealDamage / Damage or something
        if(entity_manager->has_component<Health>(c.a) && entity_manager->has_component<WrapAroundMovement>(c.b)) {
            Engine::log(" - player is hit");
            // queue_event({ Event::ShipHit, new ShipHitData { ships[si].faction }});
        }
        // asteroid to bullet => should be something like destroy on impact component or something
        else if(entity_manager->has_component<Faction>(c.a) 
            && entity_manager->has_component<WrapAroundMovement>(c.b)) {
            Engine::log(" - bullet to asteroid");
            // a is bullet
            // b is asteroid

            // destroy asteroid
            // DestroyEntityData *de = new DestroyEntityData { fe.index(i) };
            // queue_event(de);

            // queue some kind of spawn asteroid event or take care of that somewhere else
            // queue_event({ Event::AsteroidDestroyed, new AsteroidDestroyedData { 
			// 		asteroids[ai].size,
			// 		bullets[bi].faction
			// 	}});

            // destroy bullet
            // DestroyEntityData *de = new DestroyEntityData { fe.index(i) };
            // queue_event(de);

        }        
        // Find out if asteroid is hit by ship
        // -> ship is hit

        // find out if asteroid is hit by bullet
        // -> destroy bullet
        // -> destroy asteroid -> fire event
    }

    /*
    for(unsigned ai = 0; ai < asteroid_n; ++ai) {
		for(unsigned si = 0; si < ship_n; ++si) {
			Position &pp = ships[si].position;
			float pr = ships[si].radius;
			Position &ap = asteroids[ai].position;
			float ar = asteroids[ai].radius();
			if(Math::intersect_circles(pp.x, pp.y, pr, ap.x, ap.y, ar)) {
				queue_event({ Event::ShipHit, new ShipHitData { ships[si].faction }});
			}
		}
	}

	for(unsigned bi = 0; bi < bullets_n; ++bi) {
		for(unsigned ai = 0; ai < asteroid_n; ++ai) {
			Position &bp = bullets[bi].position;
			float br = bullets[bi].radius;
			Position &ap = asteroids[ai].position;
			float ar = asteroids[ai].radius();
			if(Math::intersect_circles(bp.x, bp.y, br, ap.x, ap.y, ar)) {
				queue_event({ Event::AsteroidDestroyed, new AsteroidDestroyedData { 
					asteroids[ai].size,
					bullets[bi].faction
				}});
				
				Velocity v = { asteroids[ai].velocity.x * 3, asteroids[ai].velocity.y * 3 };
				int size = asteroids[ai].size + 1;
				queue_event({ Event::SpawnAsteroid, new AsteroidSpawnData { ap, v, size } });
				v.x = -v.x;
				v.y = -v.y;
				queue_event({ Event::SpawnAsteroid, new AsteroidSpawnData { ap, v, size } });
				
				// TODO: This should be an destroy entity event and just send the ID
				bullets[bi].time_to_live = 0.0f;

				// TODO: This should be an destroy entity event and just send the ID
				// then some system could watch for destroyed asteroids and spawn new ones if needed
				// probably a part of the Event::AsteroidDestroyed
				asteroids[ai] = asteroids[asteroid_n - 1];
				asteroid_n--;
			}
		}	
	}
    */
}

inline void bullet_cleanup() {
    ComponentArray<Position> fp;
    world->fill<Velocity, Position, Faction, MoveForwardComponent>(fp);
    ComponentArray<Entity> fe;
    world->fill_entities<Velocity, Position, Faction, MoveForwardComponent>(fe);

    for(unsigned i = 0; i < fp.length; ++i) {
        const Position &p = fp.index(i);
        if(p.value.x < 0 || p.value.y < 0 || p.value.x > gw || p.value.y > gh) {
            DestroyEntityData *de = new DestroyEntityData { fe.index(i) };
            queue_event(de);
        }
    }
}

void asteroids_load() {
    Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter.data", the_sheet);

    world = make_world();
    entity_manager = world->get_entity_manager();

    // create archetype
    player_archetype = entity_manager->create_archetype<PlayerInput, Position, Velocity, Direction, Faction, WrapAroundMovement, Shield, Health, SizeComponent>();
    bullet_archetype = entity_manager->create_archetype<Position, Velocity, MoveForwardComponent, SizeComponent, Faction, ColorComponent>();
    asteroid_archetype = entity_manager->create_archetype<Position, Velocity, MoveForwardComponent, SizeComponent, WrapAroundMovement, ColorComponent>();

    spawn_player();
}

void asteroids_update() {
    system_asteroid_spawn();
	system_shield();
    system_player_input();
    system_player_movement();
    system_forward_movement();
    system_keep_in_bounds();
    system_collisions();

    bullet_cleanup();
    
    handle_events();
}

void render_debug_data() {
    int player_count = entity_manager->archetype_count(player_archetype);
    ComponentArray<PlayerInput> fpm;
    world->fill<PlayerInput, Position, Velocity, Direction>(fpm);
    std::string players = "Player entities: " + std::to_string(fpm.length) + " : " + std::to_string(player_count);
    draw_text_str(5, 5, Colors::white, players);
    
    int bullet_count = entity_manager->archetype_count(bullet_archetype);
    ComponentArray<Position> fp;
    world->fill<Position, Faction, SizeComponent, MoveForwardComponent>(fp);
    std::string bullets = "Bullet entities: " + std::to_string(fp.length) + " : " + std::to_string(bullet_count);
    draw_text_str(5, 15, Colors::white, bullets);

    int asteroid_count = entity_manager->archetype_count(asteroid_archetype);
    ComponentArray<Position> fpa;
    world->fill<Position, SizeComponent, WrapAroundMovement, ColorComponent>(fpa);
    std::string asteroids = "Asteroids entities: " + std::to_string(fpa.length) + " : " + std::to_string(asteroid_count);
    draw_text_str(5, 25, Colors::white, asteroids);
}

void asteroids_render() {
    // if(game_state.inactive) {
	// 	int seconds = (int)game_state.inactive_timer;
	// 	draw_text_font_centered(Resources::font_get("gameover"), gw / 2, gh / 2, game_state.text_color, "GAME OVER");
	// 	draw_text_font_centered(Resources::font_get("normal"), gw / 2, gh / 2 + 100, game_state.text_color, 
	// 		std::string("Resetting in: " + std::to_string(seconds) + " seconds..").c_str());
	// } else {
	//     std::string level_string = "Level: " + std::to_string(game_state.level);
	//     draw_text_centered_str(gw / 2, gh - 10, game_state.text_color, level_string);
    // }

    struct CircleRenderData : ComponentData<Position, SizeComponent, ColorComponent> {
        ComponentArray<Position> fp;
        ComponentArray<SizeComponent> fs;
        ComponentArray<ColorComponent> fc;
    } circle_data;
    world->fill_data(circle_data, circle_data.fp, circle_data.fs, circle_data.fc);
    draw_text_str(5, 45, Colors::white, "circles to render: " + std::to_string(circle_data.length));
	for(unsigned i = 0; i < circle_data.length; ++i) {
		Position &p = circle_data.fp.index(i);
        float radius = circle_data.fs.index(i).radius;
		SDL_Color c = circle_data.fc[i].color;
        // if(i == 0) {
        //     std::string clr_text = std::to_string(p.value.x) 
        //         + ", "
        //         + std::to_string(p.value.y)
        //         + ", "
        //         + std::to_string(radius)
        //         + ", "
        //         + std::to_string(circle_data.fc[i].color.r)
        //         + ", "
        //         + std::to_string(circle_data.fc[i].color.g)
        //         + ", "
        //         + std::to_string(circle_data.fc[i].color.b)
        //         + ", "
        //         + std::to_string(circle_data.fc[i].color.a);
        //     draw_text_str(5, 65, Colors::white, clr_text);
        // }
		draw_g_circe_color((int16_t)p.value.x, (int16_t)p.value.y, (int16_t)radius, c);
	}

    struct PlayerRenderData : ComponentData<Position, Direction, PlayerInput, Health, Faction, Shield> {
        ComponentArray<Position> fp;
        ComponentArray<Direction> fd;
        ComponentArray<Shield> shield;
        ComponentArray<Faction> faction;
        ComponentArray<Health> health;
    } player_data;
    world->fill_data(player_data, player_data.fp, player_data.fd, player_data.shield, player_data.faction, player_data.health);

    for(unsigned i = 0; i < player_data.length; ++i) {
        const Position &p = player_data.fp.index(i);
        const Direction &d = player_data.fd.index(i);
        const Shield &shield = player_data.shield[i];
        const Faction &faction = player_data.faction[i];
        const Health &health = player_data.health[i];
		draw_spritesheet_name_centered_rotated(the_sheet, "player", (int)p.value.x, (int)p.value.y, d.angle + 90);
		
		if(shield.is_active()) {
			draw_g_circe_RGBA((int16_t)p.value.x, (int16_t)p.value.y, 
				10, 0, 0, 255, 255);
		}
		if(shield.inactive_timer <= 0) {
			draw_g_rectangle_filled_RGBA(gw / 2 - 90, 11 + 10 * i, 5, 5, 0, 255, 0, 255);
		}
		
		std::string playerInfo = "Player " + std::to_string(faction.faction + 1) + 
			" | Lives: " + std::to_string(health.value) + " | Score: ";
		draw_text_str(gw / 2 - 80, 10 + 10 * i, game_state.text_color, playerInfo);
		draw_text_str(gw / 2 + 60, 10 + 10 * i, game_state.text_color, std::to_string(game_state.player_score_1));
	}

    render_debug_data();
}

#endif