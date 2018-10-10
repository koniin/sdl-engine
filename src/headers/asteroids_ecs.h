
// TODO:
// 1. Make a function in world that returns a struct that can index into all types you want
//      instead of making one for each
//      it also has the length/count in it and the entities
// 2. Fix faction in bullet firing in player move system
// 3. Get indexer by archetype
// 4. refactor => void set_component(const Entity entity, T component) 
// 5. too many entity allocations? void allocate(ComponentMask mask)


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
} config;

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
	float move_x;
	float move_y;
	float fire_x;
	float fire_y;
	float fire_cooldown;
	bool shield;
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

// Purely for tagging
struct MoveForwardComponent {};

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

// Application specific event queue wrapper
EventQueue event_queue;
template<typename T>
void queue_event(T *d) {
    event_queue.queue_evt(d);
}

void spawn_bullet(Vector2 position, Vector2 direction, int faction) {
    auto bullet = entity_manager->create_entity(bullet_archetype);
    Engine::logn("new bullet, x: %f, y: %f", position.x, position.y);

    // SOMETHING WITH CREATIING ENTITIES AFTER THEY HAVE BEEN DESTROYED IS WHACK

    entity_manager->set_component<Position>(bullet, { position });
    entity_manager->set_component<Faction>(bullet, { faction });
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

inline void update_player_input() {
    ComponentArray<PlayerInput> fp;
    world->init_indexer<PlayerInput>(fp);

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

inline void update_player_movement() {
    ComponentArray<PlayerInput> fpi;
    world->init_indexer<PlayerInput, Position, Velocity, Direction>(fpi);
    ComponentArray<Position> fp;
    world->init_indexer<PlayerInput, Position, Velocity, Direction>(fp);
    ComponentArray<Velocity> fv;
    world->init_indexer<PlayerInput, Position, Velocity, Direction>(fv);
    ComponentArray<Direction> fd;
    world->init_indexer<PlayerInput, Position, Velocity, Direction>(fd);

    // Engine::logn("PlayerInput: %d", fp.length);
    for(unsigned i = 0; i < fpi.length; ++i) {
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

inline void update_forward_movement() {
    ComponentArray<Velocity> fv;
    world->init_indexer<Velocity, Position, MoveForwardComponent>(fv);
    ComponentArray<Position> fp;
    world->init_indexer<Velocity, Position, MoveForwardComponent>(fp);
    
    for(unsigned i = 0; i < fp.length; ++i) {
        Velocity &velocity = fv.index(i);
        Position &position = fp.index(i);
        position.value += velocity.value;
    }
}

inline void bullet_cleanup() {
    ComponentArray<Position> fp;
    world->init_indexer<Velocity, Position, Faction, MoveForwardComponent>(fp);
    ComponentArray<Entity> fe;
    world->init_entity_indexer<Velocity, Position, Faction, MoveForwardComponent>(fe);

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
    player_archetype = entity_manager->create_archetype<PlayerInput, Position, Velocity, Direction, Faction>();
    bullet_archetype = entity_manager->create_archetype<Position, Velocity, Faction, MoveForwardComponent, SizeComponent>();

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
    Engine::logn("Position: %f", pc.value.x);
    Engine::logn("Entity: %d", e.id);
    
    spawn_bullet({ Vector2(666, 555) }, { Vector2(222, 333) }, 1);
    spawn_bullet({ Vector2(777, 444) }, { Vector2(111, 12) }, 1);

    // OLD WAY
    Engine::logn("old way:");
    ComponentArray<Position> fp;
    world->init_indexer<Velocity, Position>(fp);

    for(unsigned i = 0; i < fp.length; ++i) {
        const Position &p = fp.index(i);
        Engine::logn("Position: %f", p.value.x);
    }

    struct TestSome {
        ComponentArray<PlayerInput> fpi;
        ComponentArray<Position> fp;
        ComponentArray<Velocity> fv;
        ComponentArray<Direction> fd;
        ComponentArray<Entity> entities;
        unsigned length;
    } d;

    world->init_entity_indexer<PlayerInput, Position, Velocity, Direction>(d.entities);
    world->update(d.length, d.fpi, d.fp, d.fv, d.fd);
    Engine::logn("TestSome");
    for(unsigned i = 0; i < d.length; ++i) {
		//PlayerInput &pi = d.fpi.index(i);
        Position &position = d.fp.index(i);
        //Velocity &velocity = d.fv.index(i);
        //Direction &direction = d.fd.index(i);
        //Engine::logn("Entity: %d", d.entities.index(i));
        Engine::logn("Position: %f", position.value.x);
    }

    struct TestSomePositions {
        ComponentArray<Position> fp;
        ComponentArray<Velocity> fv;
        unsigned length;
    } dp;

    world->update<Position, Velocity>(dp.length, dp.fp, dp.fv);
    Engine::logn("TestSomePositions");
    for(unsigned i = 0; i < dp.length; ++i) {
		//PlayerInput &pi = d.fpi.index(i);
        Position &position = dp.fp.index(i);
        //Velocity &velocity = d.fv.index(i);
        //Direction &direction = d.fd.index(i);
        //Engine::logn("Entity: %d", d.entities.index(i));
        Engine::logn("Position: %f", position.value.x);
    }

    struct TestSystemCData : ComponentData<PlayerInput, Position, Velocity, Direction> {
        ComponentArray<PlayerInput> input;
        ComponentArray<Position> position;
    };

    struct TestSystemECData : EntityComponentData<PlayerInput, Position, Velocity, Direction> {
        ComponentArray<PlayerInput> input;
        ComponentArray<Position> position;
    };

    struct TestSystemPData : ComponentData<Position, Velocity> {
        ComponentArray<Position> position;
        ComponentArray<Velocity> velocity;
    };

    TestSystemCData data;
    Engine::logn("TestSystemCData update");
    world->update_data(data, data.input, data.position);
    Engine::logn("TestSystemCData: %d", data.length);
    for(unsigned i = 0; i < data.length; ++i) {
        Engine::logn("PlayerInput: %f", data.input.index(i).move_x);
        Engine::logn("Position: %f", data.position.index(i).value.x);
    }

    TestSystemECData datadata;
    Engine::logn("TestSystemECData update");
    world->update_entity_data(datadata, datadata.entities, datadata.input, datadata.position);
    Engine::logn("TestSystemECData: %d", datadata.length);
    for(unsigned i = 0; i < datadata.length; ++i) {
        Engine::logn("PlayerInput: %f", datadata.input.index(i).move_x);
        Engine::logn("Position: %f", datadata.position.index(i).value.x);
        Engine::logn("Entity: %d", datadata.entities.index(i).id);
    }

    TestSystemPData pdata;
    Engine::logn("TestSystemPData update");
    world->update_data(pdata, pdata.position, pdata.velocity);
    Engine::logn("TestSystemPData: %d", pdata.length);
    for(unsigned i = 0; i < pdata.length; ++i) {
        Engine::logn("Position: %f", pdata.position.index(i).value.x);
        Engine::logn("Velocity: %f", pdata.velocity.index(i).value.x);
    }

    //entity_manager->destroy_entity(e2);

    // SystemData<PlayerInput, Position> c;
    // c.update(world);
    // for(unsigned i = 0; i < c.length; ++i) {
    //     Engine::logn("PlayerInput: %f", c.index<PlayerInput>(i).move_x);
    //     Engine::logn("Position: %f", c.index<Position>(i).value.x);
    // }

    //world->init(c);
/*
    
*/
    //ComponentIterator<PlayerInput, Position, Velocity> cIterator;
    //world->get_iterator(cIterator);

    // MultiIndexer m;

    // ComponentData<PlayerInput, Position> d;
    // d.init(world);

    // for(unsigned i = 0; i < d.length; ++i) {
    //     Engine::logn("PlayerInput: %f", d.first.index(i).move_x);
    //     Engine::logn("Position: %f", d.second.index(i).value.x);
    // }
    
    // ForwardIndexer<Position> f;
    // world->init_indexer(f);

    // ForwardIndexer<PlayerInput> fp;
    // world->init_indexer(fp);
    // //auto c = static_cast<ComponentContainer<Position>*>(entity_manager->storage.archetypes[player_archetype.mask].components[TypeID::value<Position>()]);
    // //f.add(c->instances, c->length);

    // Engine::logn("Positions: %d", f.length);
    // for(unsigned i = 0; i < f.length; ++i) {
	// 	Position &p = f.index(i);
	// 	Engine::logn("i: %d , position: %f.0, %f.0", i, p.value.x, p.value.y);
	// }
    
}

void asteroids_update() {
    // update components
    update_player_input();
    update_player_movement();
    update_forward_movement();
    
    bullet_cleanup();
    
    handle_events();
}

void render_debug_data() {
    ComponentArray<PlayerInput> fpm;
    world->init_indexer<PlayerInput, Position, Velocity, Direction>(fpm);
    std::string players = "Player entities: " + std::to_string(fpm.length);
    draw_text_str(5, 5, Colors::white, players);
    
    ComponentArray<Position> fp;
    world->init_indexer<Position, Faction, SizeComponent>(fp);
    std::string bullets = "Bullet entities: " + std::to_string(fp.length);
    draw_text_str(5, 15, Colors::white, bullets);
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

	// for(unsigned i = 0; i < asteroid_n; ++i) {
	// 	Position &p = asteroids[i].position;
	// 	draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)asteroids[i].radius(), game_state.asteroid_color);
	// }

    ComponentArray<Position> fp;
    world->init_indexer<Position, Faction, SizeComponent>(fp);
    ComponentArray<SizeComponent> fs;
    world->init_indexer<Position, Faction, SizeComponent>(fs);
	for(unsigned i = 0; i < fp.length; ++i) {
		Position &p = fp.index(i);
        float radius = fs.index(i).radius;
		SDL_Color c = { 255, 0, 0, 255 };
		draw_g_circe_color((int16_t)p.value.x, (int16_t)p.value.y, (int16_t)radius, c);
	}

    ComponentArray<Position> fpm;
    world->init_indexer<PlayerInput, Position, Direction>(fpm);
    ComponentArray<Direction> fd;
    world->init_indexer<PlayerInput, Position, Direction>(fd);
    for(unsigned i = 0; i < fpm.length; ++i) {
        const Position &p = fpm.index(i);
        const Direction &d = fd.index(i);
		draw_spritesheet_name_centered_rotated(the_sheet, "player", (int)p.value.x, (int)p.value.y, d.angle + 90);
		
		// if(player.shield.is_active()) {
		// 	draw_g_circe_RGBA((int16_t)player.position.x, (int16_t)player.position.y, 
		// 		10, 0, 0, 255, 255);
		// }
		// if(player.shield.inactive_timer <= 0) {
		// 	draw_g_rectangle_filled_RGBA(gw / 2 - 90, 11 + 10 * i, 5, 5, 0, 255, 0, 255);
		// }
		
		// std::string playerInfo = "Player " + std::to_string(player.faction + 1) + 
		// 	" | Lives: " + std::to_string(player.health) + " | Score: ";
		// draw_text_str(gw / 2 - 80, 10 + 10 * i, game_state.text_color, playerInfo);
		// draw_text_str(gw / 2 + 60, 10 + 10 * i, game_state.text_color, std::to_string(player.score));
	}

    render_debug_data();
}

#endif