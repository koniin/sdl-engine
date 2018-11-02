#ifndef BULLET_PHYSICS_H
#define BULLET_PHYSICS_H

#include "ecs.h"
#include "engine.h"
#include "renderer.h"

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

struct ColorComponent {
    SDL_Color color;
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

// Purely for tagging
struct MoveForwardComponent {};

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

struct Configuration {
	float rotation_speed = 5.0f;
    float bullet_speed = 10.0f;
	float fire_cooldown = 0.25f; // s
} config;

struct DebugData {
    int bullets_fired = 0;
    int bullets_collided = 0;
    int bullets_to_rect = 0;
} debug_data;

World *world;
EntityManager *entity_manager;
EntityArchetype player_archetype;
EntityArchetype bullet_archetype;
EntityArchetype target_archetype;

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
    Vector2 pos = Vector2(100.0f, (float)gh / 2);
    Vector2 vel = Vector2::Zero;
    Position position = { pos };
    Velocity velocity = { vel };
    entity_manager->set_component(e, position);
    entity_manager->set_component(e, velocity);
    entity_manager->set_component<SizeComponent>(e, { 6 });
}

void spawn_bullet(Vector2 position, Vector2 direction, int faction) {
    auto bullet = entity_manager->create_entity(bullet_archetype);
    
    entity_manager->set_component<Position>(bullet, { position });
    entity_manager->set_component<ColorComponent>(bullet, { { 255, 0, 0, 255 } });
    entity_manager->set_component<Damage>(bullet, { 1 });
    
    entity_manager->set_component<Velocity>(bullet, { 
            Vector2(direction.x * config.bullet_speed, direction.y * config.bullet_speed)
        });
    entity_manager->set_component<SizeComponent>(bullet, { 4.0f });
}

void spawn_target(Position position, Velocity velocity, int size) {
    Entity asteroid = entity_manager->create_entity(target_archetype);
    entity_manager->set_component(asteroid, position);
    entity_manager->set_component(asteroid, velocity);
    entity_manager->set_component<SizeComponent>(asteroid, { 16.0f });
    entity_manager->set_component<ColorComponent>(asteroid, { Colors::white });
    entity_manager->set_component<Health>(asteroid, { 1 });
}

void handle_events() {
    for(auto &e : event_queue.events) {
		if(e.is<ShotSpawnData>()) {
            ShotSpawnData *d = e.get<ShotSpawnData>();
			spawn_bullet(d->position, d->direction, d->faction);

            debug_data.bullets_fired++;
        } 
        else if(e.is<DestroyEntityData>()) {
            DestroyEntityData *d = e.get<DestroyEntityData>();
            entity_manager->destroy_entity(d->entity);
        } 
        // else if(e.is<SpawnAsteroidData>()) {
        //     SpawnAsteroidData *d = e.get<SpawnAsteroidData>();
		// 	if(d->size <= 3)
		// 		spawn_asteroid({ d->position }, { d->velocity }, d->size);
        // }
        // e.destroy(); <- only needed if manually emptying the queue (not using clear method)
    }
    
    event_queue.clear();
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

const int rect_h = 16;
const int rect_w = 16;
static Rectangle the_square;

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
			}
        }

        if(Math::intersect_circle_AABB(first_position.x, first_position.y, first_radius, the_square)) {
            debug_data.bullets_to_rect++;
        }
    }

    for(unsigned i = 0; i < collisions.count; ++i) {
        Entity first = collisions.first[i];
        Entity second = collisions.second[i];
        if(entity_manager->has_component<Damage>(first)) {
            debug_data.bullets_collided++;
            DestroyEntityData *de = new DestroyEntityData { first };
            queue_event(de);
        }
    }
}

void bullet_load() {
    Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	
    world = make_world();
    entity_manager = world->get_entity_manager();

    // create archetype
    player_archetype = entity_manager->create_archetype<PlayerInput, Position, Velocity, Direction, SizeComponent>();
    bullet_archetype = entity_manager->create_archetype<Position, Velocity, MoveForwardComponent, SizeComponent, ColorComponent, Damage>();
    target_archetype = entity_manager->create_archetype<Position, Velocity, MoveForwardComponent, SizeComponent, ColorComponent, Health>();

    spawn_player(1);
    spawn_target({ Vector2((float)gw - 150.0f, (float)gh/2) }, Velocity(), 10);
    the_square = Rectangle(gw - 200 - (rect_h / 2), gh/2 - (rect_w / 2), 16, 16);
}

void bullet_update() {
    // Input
    {
        ComponentArray<PlayerInput> fpi;
        ComponentArray<Position> fp;
        ComponentArray<Velocity> fv;
        ComponentArray<Direction> fd;
        unsigned length;
        world->fill_by_arguments(length, fpi, fp, fv, fd);
        for(unsigned i = 0; i < fpi.length; ++i) {
            PlayerInput &pi = fpi.index(i);
            Direction &direction = fd.index(i);
            pi.fire_x = 0;
            pi.fire_y = 0;
            pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::deltaTime);
            if(Input::key_down(input_maps[0].fire)) {
                pi.fire_x = pi.fire_y = 1;
                if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
                    
                    direction.angle += pi.move_x * config.rotation_speed;
                    float rotation = direction.angle / Math::RAD_TO_DEGREE;

                    float direction_x = cos(rotation);
                    float direction_y = sin(rotation);

                    ShotSpawnData *d = new ShotSpawnData;
                    d->position = fp.index(i).value + 5.0f;
                    d->direction.x = direction_x;
                    d->direction.y = direction_y;
                    d->faction = 1;
                    queue_event(d);
                    
                    pi.fire_cooldown = config.fire_cooldown;
                }
            }

            if(Input::key_pressed(SDLK_w)) {
                config.bullet_speed += 2.0f;
            }
            if(Input::key_pressed(SDLK_s)) {
                config.bullet_speed -= 2.0f;
            }
            if(Input::key_pressed(SDLK_a)) {
                config.fire_cooldown -= 0.1f;
            }
            if(Input::key_pressed(SDLK_d)) {
                config.fire_cooldown += 0.1f;
            }
        }
    }

    // Move forward system
    {
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

    system_collisions();

    // Destroy out of bounds
    {
        ComponentArray<Position> fp;
        world->fill<Velocity, Position, Damage, MoveForwardComponent>(fp);
        ComponentArray<Entity> fe;
        world->fill_entities<Velocity, Position, Damage, MoveForwardComponent>(fe);

        for(unsigned i = 0; i < fp.length; ++i) {
            const Position &p = fp.index(i);
            if(p.value.x < 0 || p.value.y < 0 || p.value.x > gw || p.value.y > gh) {
                DestroyEntityData *de = new DestroyEntityData { fe.index(i) };
                queue_event(de);
            }
        }
    }

    handle_events();

    FrameLog::log("bullets collided: " + std::to_string(debug_data.bullets_collided));
    FrameLog::log("bullets to rect: " + std::to_string(debug_data.bullets_to_rect));
    FrameLog::log("bullets fired: " + std::to_string(debug_data.bullets_fired));
    FrameLog::log("bullet speed: " + std::to_string(config.bullet_speed));
    FrameLog::log("fire cooldown: " + std::to_string(config.fire_cooldown));
}

void bullet_render() {
    struct PlayerRenderData : ComponentData<Position, Direction, PlayerInput> {
        ComponentArray<Position> fp;
        ComponentArray<Direction> fd;
    } player_data;
    world->fill_data(player_data, player_data.fp, player_data.fd);
    for(unsigned i = 0; i < player_data.length; ++i) {
        const Position &p = player_data.fp.index(i);
        draw_g_rectangle_filled((int)p.value.x, (int)p.value.y, 16, 16, Colors::white);
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

    draw_g_rectangle_filled(the_square.x, the_square.y, rect_w, rect_h, Colors::white);

    FrameLog::render(5, 10);
}

#endif