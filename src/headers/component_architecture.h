
// Game Feel TODO:
/* ==========================

All gfx can be found in shooter_spritesheet.png
* Bullet gfx (big)
* Ship gfx
* Enemy gfx
* Muzzle flash (circular filled white first frame or something or display bullet as circle first frame)
* Bullet spread (accuracy)
* Impact effect (hit effect, like a little marker on the side we hit)
* Hit animation (Blink)
* Enemy knockback (3 pixels per frame in the direction of the bullet, would be countered by movement in normal cases)
* Leave something behind when something is killed (just destroy the hit entity, spawn something else and then respawn an enemy)
* Screen shake on fire weapon
* Screen shake on hit enemy
* player knockback on fire weapon (if player is too far back move to start pos for demo)
* Sleep on hit an enemy (20ms)
* Shells or something fly out on fire weapon (make it a "machine gun")
* BIG random explosions / explosion on kill (circle that flashes from black/grey to white to disappear for one update each)
* Smoke on explosion
* Smoke on fire gun

Do movement and then:
* Area larger than the screen with camera
* Camera lerp - follow player
* Camera towards where player is aiming
* Camera kick - move camera back on firing (moves back to player automatically if following)

Then:
* Sound and animatons
* More base in sound effects

* Gun gfx
* Gun kick - make it smaller or something when firing

*/

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
        Entity *entity;
        std::unordered_map<EntityId, unsigned> _map;
        int container_count = 0;
        int max_container_count = 0;
        void **containers;
        size_t *container_sizes;
    
        void allocate_entities(size_t count, int max_containers) {
            size = count;
            entity = new Entity[count];
            containers = new void*[max_containers];
            container_sizes = new size_t[max_containers];
            max_container_count = max_containers;
        }

        template<typename T>
        void add(T *container) {
            ASSERT_WITH_MSG(container_count < max_container_count, "Maximum container count reached, more components than containers");
            containers[container_count] = container;
            container_sizes[container_count] = sizeof(T);
            container_count++;
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

        bool contains(Entity e) {
            auto a = _map.find(e.id);
            return a != _map.end();
        }

        bool is_valid(Handle h) {
            return h.i != invalid_handle;
        }

        void create(Entity e) {
            ASSERT_WITH_MSG(length <= size, "Component storage is full, n:" + std::to_string(length));
            ASSERT_WITH_MSG(!contains(e), "Entity already has component");
            
            unsigned int index = length;
            _map[e.id] = index;
            entity[index] = e;
            length++;
        }

        void remove(Entity e) {
            if(!contains(e))
                return;

            auto a = _map.find(e.id);
            const int index = a->second;
            const unsigned lastIndex = length - 1;

            if (lastIndex >= 0) {
                // Get the entity at the index to destroy
                Entity entityToDestroy = entity[index];
                // Get the entity at the end of the array
                Entity lastEntity = entity[lastIndex];

                // Move last entity's data
                entity[index] = entity[lastIndex];

                for(int i = 0; i < container_count; i++) {
                    std::memcpy((char*)containers[i] + (index * container_sizes[i]), 
                        (char*)containers[i] + (lastIndex * container_sizes[i]), 
                        container_sizes[i]);
                    //((char*)containers[i])[index] = ((char*)containers[i])[lastIndex];
                }

                // Update map entry for the swapped entity
                _map[lastEntity.id] = index;
                // Remove the map entry for the destroyed entity
                _map.erase(entityToDestroy.id);

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
    Velocity(float xv, float yv): x(xv), y(yv) {}
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

struct Projectile : Entities::EntityData {
    Position *position;
    Velocity *velocity;

    const int radius = 8;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        allocate_entities(n, 2);
        add(position);
        add(velocity);
    }
};

struct Target : Entities::EntityData {
    Position *position;
    Velocity *velocity;

    const int radius = 8;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        allocate_entities(n, 2);
        add(position);
        add(velocity);
    }
};

template<typename T>
Position &get_position(T &entity_data, Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    return entity_data.position[handle.i];
}

template<typename T>
void set_position(T &entity_data, Entity e, Position p) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    entity_data.position[handle.i] = p;
}

template<typename T>
void set_velocity(T &entity_data, Entity e, Velocity v) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
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
Projectile projectiles;
Target targets;

Rectangle world_bounds;

struct SpawnProjectile {
    Position position;
    Velocity velocity;
};
std::vector<SpawnProjectile> projectile_queue;
void queue_projectile(Position p, Direction direction, float speed) {
    Velocity v = { direction.x * speed, direction.y * speed };
    projectile_queue.push_back({ p, v });
}
void spawn_projectiles() {
    for(size_t i = 0; i < projectile_queue.size(); i++) {
        Entity e = entity_manager.create();
        projectiles.create(e);
        set_position(projectiles, e, projectile_queue[i].position);
        set_velocity(projectiles, e, projectile_queue[i].velocity);
    }
    projectile_queue.clear();
}

void spawn_player() {
    Entity e = entity_manager.create();
    players.create(e);
    set_position(players, e, { 100, 200 });
}

void spawn_target() {
    Entity e = entity_manager.create();
    targets.create(e);
    set_position(targets, e, { 400, 200 });
    set_velocity(targets, e, { 0, 0 });
}

std::vector<Entity> entities_to_destroy;
void queue_remove_entity(Entity entity) {
    entities_to_destroy.push_back(entity);
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
            queue_projectile(players.position[i], direction, config.player_bullet_speed);
            pi.fire_cooldown = config.fire_cooldown;
        }
    }
}

template<typename T>
void move_forward(T &entityData) {
    for(int i = 0; i < entityData.length; i++) {
        entityData.position[i].x += entityData.velocity[i].x;
        entityData.position[i].y += entityData.velocity[i].y;
    }
}

template<typename T>
void keep_in_bounds(T &entityData, Rectangle &bounds) {
    for(int i = 0; i < entityData.length; i++) {
        if(entityData.position[i].x < bounds.x) { 
            entityData.position[i].x = (float)bounds.x; 
        }
        if(entityData.position[i].x > bounds.right()) { 
            entityData.position[i].x = (float)bounds.right(); 
        }
        if(entityData.position[i].y < bounds.y) { 
            entityData.position[i].y = (float)bounds.y; 
        }
        if(entityData.position[i].y > bounds.bottom()) { 
            entityData.position[i].y = (float)bounds.bottom(); 
        }
    } 
}

template<typename T>
void remove_out_of_bounds(T &entityData, Rectangle &bounds) {
    for(int i = 0; i < entityData.length; i++) {
        if(!bounds.contains((int)entityData.position[i].x, (int)entityData.position[i].y)) {
            Engine::logn("Queueing destroy entity: %d, at pos: %d, %d", entityData.entity[i].id, (int)entityData.position[i].x, (int)entityData.position[i].y);
            queue_remove_entity(entityData.entity[i]);
        }
    } 
}

void system_player_drag() {
    for(int i = 0; i < players.length; i++) {
        Velocity &velocity = players.velocity[i];
	    velocity.x = velocity.x - velocity.x * config.drag;
	    velocity.y = velocity.y - velocity.y * config.drag;
    }
}

void system_move() {
    move_forward(players);
    keep_in_bounds(players, world_bounds);
    move_forward(targets);
    keep_in_bounds(targets, world_bounds);
    move_forward(projectiles);
    remove_out_of_bounds(projectiles, world_bounds);
    system_player_drag();
}

struct CollisionPairs {
    Entity *first;
    Entity *second;

    unsigned int count = 0;

    void allocate(size_t size) {
        first = new Entity[size];
        second = new Entity[size];
    }

    void push(Entity a, Entity b) {
        first[count] = a;
        second[count] = b;
        ++count;
    }

    void clear() {
        count = 0;
    }
};

void system_collisions(CollisionPairs &collision_pairs) {
    // struct CollisionGroup : EntityComponentData<Position, SizeComponent> {
    //     ComponentArray<Position> position;
    //     ComponentArray<SizeComponent> collision_data;
    // };
    // CollisionGroup a, b;
    // world->fill_entity_data(a, a.entities, a.position, a.collision_data);
    // world->fill_entity_data(b, b.entities, b.position, b.collision_data);
    for(int i = 0; i < projectiles.length; ++i) {
        for(int j = 0; j < targets.length; ++j) {
            const Position &first_position = projectiles.position[i];
            const Position &second_position = targets.position[i];
            const float first_radius = (float)projectiles.radius;
            const float second_radius = (float)targets.radius;
            if(Math::intersect_circles(first_position.x, first_position.y, first_radius, second_position.x, second_position.y, second_radius)) {
                collision_pairs.push(projectiles.entity[i], targets.entity[j]);
            }
        }
    }

    for(unsigned i = 0; i < collision_pairs.count; ++i) {
        Entity first = collision_pairs.first[i];
        // Entity second = collisions.second[i];
        queue_remove_entity(first);
    }
    collision_pairs.clear();

    // collisions.clear();
    // for(unsigned i = 0; i < a.length; ++i) {
    //     const Vector2 first_position = a.position[i].value;
    //     const float first_radius = a.collision_data[i].radius;
    //     const Entity first_entity = a.entities[i];
    //     for(unsigned j = 0; j < b.length; ++j) {
    //         const Vector2 second_position = b.position[j].value;
    //         const float second_radius = b.collision_data[j].radius;
    //         const Entity second_entity = b.entities[j];
    //         if(i != j 
    //             && Math::intersect_circles(first_position.x, first_position.y, first_radius, 
    //                 second_position.x, second_position.y, second_radius)) {
    //             collisions.push(first_entity, second_entity);
	// 		}
    //     }

    //     if(Math::intersect_circle_AABB(first_position.x, first_position.y, first_radius, the_square)) {
    //         debug_data.bullets_to_rect++;
    //     }
    // }

    // for(unsigned i = 0; i < collisions.count; ++i) {
    //     Entity first = collisions.first[i];
    //     Entity second = collisions.second[i];
    //     if(entity_manager->has_component<Damage>(first)) {
    //         debug_data.bullets_collided++;
    //         DestroyEntityData *de = new DestroyEntityData { first };
    //         queue_event(de);
    //     }
    // }
}

void remove_destroyed_entities() {
    for(size_t i = 0; i < entities_to_destroy.size(); i++) {
        Engine::logn("destroying: %d", entities_to_destroy[i].id);
        players.remove(entities_to_destroy[i]);
        projectiles.remove(entities_to_destroy[i]);
        targets.remove(entities_to_destroy[i]);
        entity_manager.destroy(entities_to_destroy[i]);
    }
    entities_to_destroy.clear();
}

static CollisionPairs collisions;
static SpriteSheet the_sheet;
void load_arch() {
    Engine::set_base_data_folder("data");
	Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter_spritesheet.data", the_sheet);

    world_bounds = { 0, 0, (int)gw, (int)gh };

    players.allocate(2);
    projectiles.allocate(128);
    targets.allocate(128);
    projectile_queue.reserve(64);
    entities_to_destroy.reserve(64);
    collisions.allocate(128);

    spawn_player();
    spawn_target();
}

void update_arch() {
    FrameLog::reset();

    system_player_get_input();
    system_player_handle_input();
    system_move();
    system_collisions(collisions);
    spawn_projectiles();

    remove_destroyed_entities();
    
    FrameLog::log("Players: " + std::to_string(players.length));
    FrameLog::log("Projectiles: " + std::to_string(projectiles.length));
    FrameLog::log("Targets: " + std::to_string(targets.length));
}

void render_arch() {
    for(int i = 0; i < players.length; i++) {
        Position &p = players.position[i];
        Direction &d = players.direction[i];
        draw_spritesheet_name_centered_rotated(the_sheet, "player_1", (int)p.x, (int)p.y, d.angle + 90);
        // SDL_Color c = Colors::white;
        // draw_g_circe_color((int16_t)p.x, (int16_t)p.y, 4, c);
    }
    
    for(int i = 0; i < projectiles.length; ++i) {
		Position &p = projectiles.position[i];
		SDL_Color c = { 0, 255, 0, 255 };
		draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)projectiles.radius, c);
	}

    for(int i = 0; i < targets.length; ++i) {
		Position &p = targets.position[i];
		SDL_Color c = { 255, 0, 0, 255 };
		draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)targets.radius, c);
	}

    FrameLog::render(5, 5);
}

#endif