#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

#include "engine.h"
#include "renderer.h"
#include <queue>

const unsigned ENTITY_INDEX_BITS = 22;
const unsigned ENTITY_INDEX_MASK = (1<<ENTITY_INDEX_BITS)-1;

const unsigned ENTITY_GENERATION_BITS = 8;
const unsigned ENTITY_GENERATION_MASK = (1<<ENTITY_GENERATION_BITS)-1;

typedef unsigned EntityId;
struct Entity {
    EntityId id;

    unsigned index() const { return id & ENTITY_INDEX_MASK; }
    unsigned generation() const { return (id >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK; }
};

const unsigned MINIMUM_FREE_INDICES = 1024;

struct EntityManager {
    std::vector<unsigned char> _generation;
    std::queue<unsigned> _free_indices;

    Entity create() {
        unsigned idx;
        if (_free_indices.size() > MINIMUM_FREE_INDICES) {
            idx = _free_indices.front();
            _free_indices.pop();
        } else {
            _generation.push_back(0);
            idx = _generation.size() - 1;
            ASSERT_WITH_MSG(idx < (1 << ENTITY_INDEX_BITS), "idx is malformed, larger than 22 bits?");
        }

        return make_entity(idx, _generation[idx]);
    }

    Entity make_entity(unsigned idx, unsigned char generation) {
        Entity e;
        auto id = generation << ENTITY_INDEX_BITS | idx;
        e.id = id;
        return e;
    }

    bool alive(Entity e) const {
        return _generation[e.index()] == e.generation();
    }

    void destroy(Entity e) {
        if(!alive(e))
            return;

        const unsigned idx = e.index();
        ++_generation[idx];
        _free_indices.push(idx);
    }
};

namespace Entities {
    struct EntityData {
        int length;
        int size;
        EntityId *entity;
        std::unordered_map<EntityId, unsigned> _map;
        int container_count = 0;
        int max_container_count = 0;
        void **containers;
    
        void allocate_entities(size_t count, int max_containers) {
            size = count;
            entity = new EntityId[count];
            containers = new void*[max_containers];
            max_container_count = max_containers;
        }

        void add(void *container) {
            ASSERT_WITH_MSG(container_count < max_container_count, "Maximum container count reached, more components than containers");
            containers[container_count++] = container;
        }

        static const int invalid_handle = -1;

        struct Handle {
            int i = -1;
        };

        Handle get_handle(Entity e) {
            auto a = _map.find(e.id);
            if(a != _map.end()) {
                return { (int)a->second };
            }
            return { invalid_handle };
        }

        bool is_alive(Entity e) {
            auto a = _map.find(e.id);
            return a != _map.end();
        }

        bool is_valid(Handle h) {
            return h.i != invalid_handle;
        }

        void create(Entity e) {
            ASSERT_WITH_MSG(length <= size, "Component storage is full, n:" + std::to_string(length));
            ASSERT_WITH_MSG(!is_alive(e), "Entity already has component");
            
            unsigned int index = length;
            _map[e.id] = index;
            length++;
        }

        void remove_component(Entity e) {
            if(!is_alive(e))
                return;

            auto a = _map.find(e.id);
            const int index = a->second;
            const unsigned lastIndex = length - 1;

            if (lastIndex >= 0) {
                // Get the entity at the index to destroy
                EntityId entityToDestroy = entity[index];
                // Get the entity at the end of the array
                EntityId lastEntity = entity[lastIndex];

                // Move last entity's data
                entity[index] = entity[lastIndex];

                for(int i = 0; i < container_count; i++) {
                    ((char*)containers[i])[index] = ((char*)containers[i])[lastIndex];
                }

                // Update map entry for the swapped entity
                _map[lastEntity] = index;
                // Remove the map entry for the destroyed entity
                _map.erase(entityToDestroy);

                // Decrease count
                length--;
            }
        }
    };
};

struct PlayerInput {
	// Input
	float move_x;
	float move_y;
	float fire_x;
	float fire_y;
	float fire_cooldown;
	bool shield;

    PlayerInput() {
        fire_cooldown = 0.0f;
        shield = false;
    }
};

struct Position {
    float x, y;
};

struct Velocity {
    float x, y;

    Velocity(): x(0), y(0) {}
};

struct Direction {
    float x, y;
    float angle;

    Direction() {
        x = y = angle = 0.0f;
    }
};

struct CollisionData {
    float radius;
};

struct Player : Entities::EntityData {
    Position *position;
    Velocity *velocity;
    Direction *direction;
    PlayerInput *input;

    void allocate(size_t pn) {
        position = new Position[pn];
        velocity = new Velocity[pn];
        direction = new Direction[pn];
        input = new PlayerInput[pn];
        allocate_entities(pn, 4);
        add(position);
        add(velocity);
        add(direction);
        add(input);
    }
};

struct Bullet : Entities::EntityData {
    Position *position;
    Velocity *velocity;

    const int radius = 8;

    struct SpawnBullet {
        Position position;
        Velocity velocity;
    };
    std::vector<SpawnBullet> bullet_queue;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        allocate_entities(n, 2);
        add(position);
        add(velocity);

        bullet_queue.reserve(64);
    }
    
    void spawn_bullet(Position p, Direction direction, float speed) {
        Velocity v;
        v.x = direction.x * speed;
        v.y = direction.y * speed;
        bullet_queue.push_back({ p, v });
    }
};

template<typename T>
Position &get_position(T &entity_data, Entity e) {
    ASSERT_WITH_MSG(entity_data.is_alive(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    return entity_data.position[handle.i];
}

template<typename T>
void set_position(T &entity_data, Entity e, Position p) {
    ASSERT_WITH_MSG(entity_data.is_alive(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    entity_data.position[handle.i] = p;
}

template<typename T>
void set_velocity(T &entity_data, Entity e, Velocity v) {
    ASSERT_WITH_MSG(entity_data.is_alive(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    entity_data.velocity[handle.i] = v;
}

/*
const int MAX_ENEMIES = 128;
struct Enemy {
    int n;
    int size;
    EntityId entity[MAX_ENEMIES];
    Position positions[MAX_ENEMIES];
    Velocity velocity[MAX_ENEMIES];
    std::unordered_map<EntityId, unsigned> _map;
};
*/

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
	float bullet_time_to_live = 20.5f; // really high because we don't care
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

EntityManager entity_manager;
Player players;
Bullet bullets;

void spawn_player() {
    Entity e = entity_manager.create();
    players.create(e);
    set_position(players, e, { 100, 200 });
}

static SpriteSheet the_sheet;
void load_arch() {
    Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter.data", the_sheet);

    players.allocate(2);
    bullets.allocate(128);
    spawn_player();
}

void system_player_get_input() {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        pi.move_x = 0;
        pi.move_y = 0;
        pi.fire_x = 0;
        pi.fire_y = 0;
        pi.shield = false;

        InputMapping key_map = input_maps[i];

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

void system_player_handle_input() {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        Velocity &velocity = players.velocity[i];
        Direction &direction = players.direction[i];
        
        // Update rotation based on rotational speed
        // for other objects than player input once
        direction.angle += pi.move_x * config.rotation_speed;
        float rotation = direction.angle / Math::RAD_TO_DEGREE;
        direction.x = cos(rotation);
        direction.y = sin(rotation);
        
	    velocity.x += direction.x * pi.move_y * config.acceleration;
	    velocity.y += direction.y * pi.move_y * config.acceleration;

        
        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            bullets.spawn_bullet(players.position[i], direction, config.player_bullet_speed);

            // d->position = position;
            // d->rotation.x = direction_x;
            // d->rotation.y = direction_y;
            // d->time_to_live = config.bullet_time_to_live;
            // d->faction = sdata.faction;
            // e.data = d;
            // queue_event(e);
            pi.fire_cooldown = config.fire_cooldown;
        }
    }
}

template<typename T>
void update_positions(T &entityData) {
    for(int i = 0; i < entityData.length; i++) {
        entityData.position[i].x += entityData.velocity[i].x;
        entityData.position[i].y += entityData.velocity[i].y;
    }
}

void system_move() {
    update_positions(players);
    update_positions(bullets);
}

void system_player_drag() {
    for(int i = 0; i < players.length; i++) {
        Velocity &velocity = players.velocity[i];
	    // Use Stokes' law to apply drag to the object
	    velocity.x = velocity.x - velocity.x * config.drag;
	    velocity.y = velocity.y - velocity.y * config.drag;
    }
}

void system_spawn() {
    for(size_t i = 0; i < bullets.bullet_queue.size(); i++) {
        Entity e = entity_manager.create();
        bullets.create(e);
        set_position(bullets, e, bullets.bullet_queue[i].position);
        set_velocity(bullets, e, bullets.bullet_queue[i].velocity);
    }
    bullets.bullet_queue.clear();
}

void update_arch() {
    system_player_get_input();
    system_player_handle_input();
    system_move();
    system_player_drag();

    system_spawn();
}

void render_arch() {
    for(int i = 0; i < players.length; i++) {
        Position &p = players.position[i];
        Direction &d = players.direction[i];
        draw_spritesheet_name_centered_rotated(the_sheet, "player", (int)p.x, (int)p.y, d.angle + 90);
        // SDL_Color c = Colors::white;
        // draw_g_circe_color((int16_t)p.x, (int16_t)p.y, 4, c);
    }
    
    for(int i = 0; i < bullets.length; ++i) {
		Position &p = bullets.position[i];
		SDL_Color c = { 255, 0, 0, 255 };
		draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)bullets.radius, c);
	}
}

#endif