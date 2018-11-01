#ifndef ASTEROIDS_ECS_H
#define ASTEROIDS_ECS_H

#include "ecs.h"

struct AsteroidsConfig {
	float rotation_speed = 5.0f; 
	float acceleration = 0.2f;
	float brake_speed = -0.05f;
	float drag = 0.02f;
	float fire_cooldown = 0.25f; // s
	float player_bullet_speed = 20;
	float player_bullet_size = 1;
	int player_faction_1 = 0;
	int player_faction_2 = 1;
    int asteroid_faction = 10;
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
void game_state_inactivate();
void game_state_reset();

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

    // This could be it's own component
    bool can_be_invulnerable = false;
    float invulnerability_on_hit = 0;
    bool is_invulnerable() {
        return invulnerability_on_hit > 0;
    }
};

struct Damage {
    int value = 0;
    bool destroy_on_impact = true;
};

struct PointComponent {
    int value;
};

struct Shield {
	float active_timer = 0;
	float inactive_timer = 0;
	bool is_active() const {
		return active_timer > 0.0f;
	}
};

struct SplitOnDeath {
    int pieces;
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

struct SpawnAsteroidData {
    Vector2 position;
    Vector2 velocity;
    int size;
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

void queue_clear() {
    event_queue.clear();
}

void spawn_player(int faction) {
    auto e = entity_manager->create_entity(player_archetype);
    
    // Set some component data
    entity_manager->set_component<PlayerInput>(e, { 0, 0, 0, 0, 0, false });
    entity_manager->set_component<Direction>(e, { 0 });
    Vector2 pos = Vector2((float)gw / 2, (float)gh / 2);
    Vector2 vel = Vector2::Zero;
    Position position = { pos };
    Velocity velocity = { vel };
    entity_manager->set_component(e, position);
    entity_manager->set_component(e, velocity);
    entity_manager->set_component<Health>(e, { 3, true });
    entity_manager->set_component<SizeComponent>(e, { 7 });
    entity_manager->set_component<Faction>(e, { faction });
}

void spawn_bullet(Vector2 position, Vector2 direction, int faction) {
    auto bullet = entity_manager->create_entity(bullet_archetype);
    
    entity_manager->set_component<Position>(bullet, { position });
    entity_manager->set_component<Faction>(bullet, { faction });
    entity_manager->set_component<ColorComponent>(bullet, { config.bullet_color });
    entity_manager->set_component<Damage>(bullet, { 1 });
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

float asteroid_radius(const int size) {
    if(size == 1)
		return 16.0f;
	else if(size == 2) 
		return 8.0f;
	else 
		return 4.0f;
}

int asteroid_size(const float radius) {
    if(radius == 16.0f)
		return 1;
	else if(radius == 8.0f) 
		return 2;
	else 
		return 3;
}

void spawn_asteroid(Position position, Velocity velocity, int size) {
    Entity asteroid = entity_manager->create_entity(asteroid_archetype);
    entity_manager->set_component(asteroid, position);
    entity_manager->set_component(asteroid, velocity);
    entity_manager->set_component<SizeComponent>(asteroid, { asteroid_radius(size) });
    entity_manager->set_component<ColorComponent>(asteroid, { config.asteroid_color });
    entity_manager->set_component<Health>(asteroid, { 1 });
    entity_manager->set_component<Damage>(asteroid, { 1, false });
    entity_manager->set_component<Faction>(asteroid, { config.asteroid_faction });
    entity_manager->set_component<SplitOnDeath>(asteroid, { 2 });
    int score = 0;
    switch(size) {
		case 1: score = 10; break;
		case 2: score = 20; break;
		case 3: score = 50; break;
	}
    entity_manager->set_component<PointComponent>(asteroid, { score });

    // Engine::logn("spawning asteroid: x: %f, y: %f, size: %d, radius: %f",
    //     position.value.x, position.value.y, size, asteroid_radius(size));
}

void system_asteroid_spawn() {
    if(game_state.inactive) {
        return;
    }

    size_t asteroids = entity_manager->archetype_count(asteroid_archetype);
	if(asteroids == 0) {
		game_state.level++;
		for(int i = 0; i < game_state.level + config.asteroid_count_increase_per_level; ++i) {
            Position position = { Vector2(RNG::range_f(0, (float)gw), RNG::range_f(0, (float)gh)) };
            Velocity velocity = { Vector2(RNG::range_f(0, 100) / 100.0f - 0.5f, RNG::range_f(0, 100) / 100.0f - 0.5f) };
            int new_asteroid_size = 1;
            spawn_asteroid(position, velocity, new_asteroid_size);
        }
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
    ComponentArray<Faction> ff;
    unsigned length;
    world->fill_by_arguments(length, fpi, fp, fv, fd, ff);
    
    // Engine::logn("PlayerInput: %d", fp.length);
    for(unsigned i = 0; i < length; ++i) {
		PlayerInput &pi = fpi.index(i);
        Position &position = fp.index(i);
        Velocity &velocity = fv.index(i);
        Direction &direction = fd.index(i);
        const Faction faction = ff.index(i);

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
            d->faction = faction.faction;
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

struct CollisionData {
    std::vector<Entity> first;
    std::vector<Entity> second;

    unsigned int count = 0;

    void push(Entity a, Entity b) {
        first.push_back(a);
        second.push_back(b);
        ++count;
    }

    void clear() {
        count = 0;
        first.clear();
        second.clear();
    }
} collisions;

void system_collisions() {
    struct CollisionGroup : EntityComponentData<Position, SizeComponent> {
        ComponentArray<Position> position;
        ComponentArray<SizeComponent> collision_data;
    };
    CollisionGroup a, b;
    world->fill_entity_data(a, a.entities, a.position, a.collision_data);
    world->fill_entity_data(b, b.entities, b.position, b.collision_data);
    
    collisions.clear();
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
                collisions.push(first_entity, second_entity);
                //queue_event({ Event::ShipHit, new ShipHitData { ships[si].faction }});
			}
        }
        // b.position.reset();
        // b.collision_data.reset();
        // b.entities.reset();
    }
}

inline void system_damage() {
    for(unsigned i = 0; i < collisions.count; ++i) {
        Entity first = collisions.first[i];
        Entity second = collisions.second[i];
        if(entity_manager->has_component<Health>(first) && entity_manager->has_component<Damage>(second)
            && entity_manager->has_component<Faction>(first) && entity_manager->has_component<Faction>(second)) {
                const Faction faction_a = entity_manager->get_component<Faction>(first);
                const Faction faction_b = entity_manager->get_component<Faction>(second);
                if(faction_a.faction != faction_b.faction) {
                    Health &health = entity_manager->get_component<Health>(first);
                    const Damage damage = entity_manager->get_component<Damage>(second);

                    bool has_shield = entity_manager->has_component<Shield>(first);
                    if(!has_shield || has_shield && !entity_manager->get_component<Shield>(first).is_active()) {
                        if(!health.is_invulnerable()) {
                            health.value -= damage.value;
                            // Could be another component so we don't need this, or a setting on the component
                            if(health.can_be_invulnerable) {
                                health.invulnerability_on_hit = config.player_shield_time;
                            }
                        }
                    }

                    if(damage.destroy_on_impact) {
                        DestroyEntityData *de = new DestroyEntityData { second };
                        queue_event(de);
                    }
                }
            
        }
    }
}

void system_health_split(Entity entity) {
    // This is not really how it's supposed to work
    // but the fact that only asteroids split is known
    // we can use that to spawn new ones
    const Velocity velocity = entity_manager->get_component<Velocity>(entity);
    const Position position = entity_manager->get_component<Position>(entity);
    const SizeComponent size_component = entity_manager->get_component<SizeComponent>(entity);
    Vector2 v = Vector2(velocity.value.x * 3, velocity.value.y * 3);
	int size = asteroid_size(size_component.radius) + 1;
	if(size <= 3) {
        queue_event(new SpawnAsteroidData { position.value, v, size });
        v.x = -v.x;
	    v.y = -v.y;
        queue_event(new SpawnAsteroidData { position.value, v, size });
    }
}

void system_health() {
    ComponentArray<Health> fh;
    world->fill<Health>(fh);
    ComponentArray<Entity> fe;
    world->fill_entities<Health>(fe);
    for(unsigned i = 0; i < fh.length; i++) {
        fh[i].invulnerability_on_hit = Math::max_f(0.0f, fh[i].invulnerability_on_hit - Time::deltaTime);
        
        if(fh[i].value <= 0) {
            DestroyEntityData *de = new DestroyEntityData { fe[i] };
            queue_event(de);

            if(entity_manager->has_component<PointComponent>(fe[i])) {
                const PointComponent p = entity_manager->get_component<PointComponent>(fe[i]);
                game_state.player_score_1 += p.value;
            }
            if(entity_manager->has_component<SplitOnDeath>(fe[i])) {
                system_health_split(fe[i]);
            } 
            if(entity_manager->has_component<PlayerInput>(fe[i])) {
                Engine::logn("player destroyed.");
                // reset game state with inactive time etc
                Engine::logn("Inactivate game and reset after time");
                game_state_inactivate();
            }
        }
    }
}

inline void system_out_of_bounds() {
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

void handle_events() {
    for(auto &e : event_queue.events) {
		if(e.is<ShotSpawnData>()) {
            ShotSpawnData *d = e.get<ShotSpawnData>();
			spawn_bullet(d->position, d->direction, d->faction);
        } else if(e.is<DestroyEntityData>()) {
            DestroyEntityData *d = e.get<DestroyEntityData>();
            entity_manager->destroy_entity(d->entity);
        } else if(e.is<SpawnAsteroidData>()) {
            SpawnAsteroidData *d = e.get<SpawnAsteroidData>();
			if(d->size <= 3)
				spawn_asteroid({ d->position }, { d->velocity }, d->size);
        }
        // e.destroy(); <- only needed if manually emptying the queue (not using clear method)
    }
    
    event_queue.clear();
}

void game_state_reset() {
    spawn_player(config.player_faction_1);
	// spawn_player(config.player_faction_2);
	game_state.level = 0;
	
    Engine::logn("reset!");
}

void game_state_inactivate() {
    game_state.inactive = true;
	game_state.inactive_timer = game_state.pause_time;
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
    bullet_archetype = entity_manager->create_archetype<Position, Velocity, MoveForwardComponent, SizeComponent, Faction, ColorComponent, Damage>();
    asteroid_archetype = entity_manager->create_archetype<Position, Velocity, MoveForwardComponent, SizeComponent, Faction, WrapAroundMovement, ColorComponent, Health, Damage, PointComponent, SplitOnDeath>();

    game_state_reset();
}

void asteroids_update() {
    if(game_state.inactive) {
		game_state.inactive_timer -= Time::deltaTime;
		// Remove all asteroids and bullets and ships
        world->remove_all<MoveForwardComponent>();
        queue_clear();
		if(game_state.inactive_timer <= 0.0f) {
			game_state_reset();
            game_state.inactive = false;
		}
	}

    system_asteroid_spawn();
	system_shield();
    system_player_input();
    system_player_movement();
    system_forward_movement();
    system_keep_in_bounds();
    system_collisions();
    system_damage();
    system_health();
    system_out_of_bounds();
    
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
    unsigned bullets_by_archetype = 0;
    world->fill_by_archetype_exact(bullet_archetype, bullets_by_archetype, fp);
    std::string bullets = "Bullet entities: " + std::to_string(fp.length) + " : " + std::to_string(bullet_count);
    draw_text_str(5, 15, Colors::white, bullets);

    int asteroid_count = entity_manager->archetype_count(asteroid_archetype);
    ComponentArray<Position> fpa;
    world->fill<Position, SizeComponent, WrapAroundMovement, ColorComponent>(fpa);
    std::string asteroids = "Asteroids entities: " + std::to_string(fpa.length) + " : " + std::to_string(asteroid_count);
    draw_text_str(5, 25, Colors::white, asteroids);

    
    struct CircleRenderData : ComponentData<Position, SizeComponent, ColorComponent> {
        ComponentArray<Position> fp;
        ComponentArray<SizeComponent> fs;
        ComponentArray<ColorComponent> fc;
    } circle_data;
    world->fill_data(circle_data, circle_data.fp, circle_data.fs, circle_data.fc);
    draw_text_str(5, 45, Colors::white, "circles to render: " + std::to_string(circle_data.length));

    FrameLog::render(5, 60);
}

void asteroids_render() {
    if(game_state.inactive) {
		int seconds = (int)game_state.inactive_timer;
		draw_text_font_centered(Resources::font_get("gameover"), gw / 2, gh / 2, game_state.text_color, "GAME OVER");
		draw_text_font_centered(Resources::font_get("normal"), gw / 2, gh / 2 + 100, game_state.text_color, 
			std::string("Resetting in: " + std::to_string(seconds) + " seconds..").c_str());
	} else {
	    std::string level_string = "Level: " + std::to_string(game_state.level);
	    draw_text_centered_str(gw / 2, gh - 10, game_state.text_color, level_string);
    }

    struct CircleRenderData : ComponentData<Position, SizeComponent, ColorComponent> {
        ComponentArray<Position> fp;
        ComponentArray<SizeComponent> fs;
        ComponentArray<ColorComponent> fc;
    } circle_data;
    world->fill_data(circle_data, circle_data.fp, circle_data.fs, circle_data.fc);
	for(unsigned i = 0; i < circle_data.length; ++i) {
		Position &p = circle_data.fp.index(i);
        float radius = circle_data.fs.index(i).radius;
		SDL_Color c = circle_data.fc[i].color;
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