
// Game Feel TODO:
/* ==========================

All gfx can be found in shooter_spritesheet.png
[X] Bullet gfx (big)
[X] Ship gfx
[X] Enemy gfx
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
[ ] Player ship rotation animation (exists in sheet)
* More base in sound effects

* Gun gfx
* Gun kick - make it smaller or something when firing

*/

#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

#include "engine.h"
#include "renderer.h"
#include <queue>

namespace ECS {
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

struct SpriteComponent {
    float scale;
    float rotation;
    int16_t color_r;
    int16_t color_g;
    int16_t color_b;
    int16_t color_a;
    size_t sprite_sheet_index;
    std::string sprite_name;
    int layer;

    SpriteComponent() {}

    SpriteComponent(size_t sprite_sheet, std::string name) : sprite_sheet_index(sprite_sheet), sprite_name(name) {
        scale = 1.0f;
        rotation = 1.0f;
        color_r = color_g = color_b = color_a = 255;
        layer = 0;
    }
};

struct CollisionData {
    float radius;
};

struct Player : ECS::EntityData {
    Position *position;
    Velocity *velocity;
    Direction *direction;
    PlayerInput *input;
    SpriteComponent *sprite;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        direction = new Direction[n];
        input = new PlayerInput[n];
        sprite = new SpriteComponent[n];

        allocate_entities(n, 5);

        add(position);
        add(velocity);
        add(direction);
        add(input);
        add(sprite);
    }
};

struct Projectile : ECS::EntityData {
    Position *position;
    Velocity *velocity;
    SpriteComponent *sprite;

    const int radius = 8;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        sprite = new SpriteComponent[n];

        allocate_entities(n, 3);

        add(position);
        add(velocity);
        add(sprite);
    }
};

struct Target : ECS::EntityData {
    Position *position;
    Velocity *velocity;
    SpriteComponent *sprite;

    const int radius = 8;

    // Here you can put a list of base data for different targets

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        sprite = new SpriteComponent[n];

        allocate_entities(n, 3);

        add(position);
        add(velocity);
        add(sprite);
    }
};

struct Effect : ECS::EntityData {
    Position *position;
    Velocity *velocity;

    void allocate(size_t n) {
        position = new Position[n];
        velocity = new Velocity[n];
        allocate_entities(n, 2);
        add(position);
        add(velocity);
    }
};

template<typename T>
Position &get_position(T &entity_data, ECS::Entity e) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    return entity_data.position[handle.i];
}

template<typename T>
void set_position(T &entity_data, ECS::Entity e, Position p) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    entity_data.position[handle.i] = p;
}

template<typename T>
void set_velocity(T &entity_data, ECS::Entity e, Velocity v) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    entity_data.velocity[handle.i] = v;
}

template<typename T>
void set_sprite(T &entity_data, ECS::Entity e, SpriteComponent s) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);
    entity_data.sprite[handle.i] = s;
}

// Pixels per frame
constexpr float player_bullet_speed() {
    return 8.0f / 0.016667f;
}

constexpr float player_move_acceleration() {
    return 10.0f / 0.016667f;
}

struct PlayerConfiguration {
	float rotation_speed = 3.0f;
	float move_acceleration = player_move_acceleration();
	float drag = 0.04f;
	float fire_cooldown = 0.15f; // s
	float bullet_speed = player_bullet_speed();
} player_config;

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

ECS::EntityManager entity_manager;
Player players;
Projectile projectiles;
Target targets;
Effect effects;

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
        auto e = entity_manager.create();
        projectiles.create(e);
        set_position(projectiles, e, projectile_queue[i].position);
        set_velocity(projectiles, e, projectile_queue[i].velocity);
        SpriteComponent s = SpriteComponent(0, "bullet_2");
        set_sprite(projectiles, e, s);
    }
    projectile_queue.clear();
}

void spawn_player() {
    auto e = entity_manager.create();
    players.create(e);
    set_position(players, e, { 100, 200 });
    SpriteComponent s = SpriteComponent(0, "player_1");
    s.layer = 1;
    set_sprite(players, e, s);
}

void spawn_target() {
    auto e = entity_manager.create();
    targets.create(e);
    set_position(targets, e, { 400, 200 });
    set_velocity(targets, e, { 0, 0 });
    SpriteComponent s = SpriteComponent(0, "enemy_1");
    set_sprite(targets, e, s);
}

std::vector<ECS::Entity> entities_to_destroy;
void queue_remove_entity(ECS::Entity entity) {
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
        direction.angle += pi.move_x * player_config.rotation_speed;
        float rotation = direction.angle / Math::RAD_TO_DEGREE;
        direction.x = cos(rotation);
        direction.y = sin(rotation);
        
	    velocity.x += direction.x * pi.move_y * player_config.move_acceleration * Time::deltaTime;
	    velocity.y += direction.y * pi.move_y * player_config.move_acceleration * Time::deltaTime;

        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            queue_projectile(players.position[i], direction, player_config.bullet_speed);
            pi.fire_cooldown = player_config.fire_cooldown;
        }
    }
}

template<typename T>
void move_forward(T &entityData) {
    for(int i = 0; i < entityData.length; i++) {
        entityData.position[i].x += entityData.velocity[i].x * Time::deltaTime;
        entityData.position[i].y += entityData.velocity[i].y * Time::deltaTime;
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
            queue_remove_entity(entityData.entity[i]);
        }
    } 
}

void system_player_drag() {
    for(int i = 0; i < players.length; i++) {
        Velocity &velocity = players.velocity[i];
	    velocity.x = velocity.x - velocity.x * player_config.drag;
	    velocity.y = velocity.y - velocity.y * player_config.drag;
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
    ECS::Entity *first;
    ECS::Entity *second;

    unsigned int count = 0;

    void allocate(size_t size) {
        first = new ECS::Entity[size];
        second = new ECS::Entity[size];
    }

    void push(ECS::Entity a, ECS::Entity b) {
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
            const Position &second_position = targets.position[j];
            const float first_radius = (float)projectiles.radius;
            const float second_radius = (float)targets.radius;
            if(Math::intersect_circles(first_position.x, first_position.y, first_radius, second_position.x, second_position.y, second_radius)) {
                collision_pairs.push(projectiles.entity[i], targets.entity[j]);
            }

            // if(Math::intersect_circle_AABB(first_position.x, first_position.y, first_radius, the_square))
        }
    }

    for(unsigned i = 0; i < collision_pairs.count; ++i) {
        //ECS::Entity first = ;
        // Entity second = collisions.second[i];
        queue_remove_entity(collision_pairs.first[i]);
    }
    collision_pairs.clear();
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

static std::vector<SpriteSheet> sprite_sheets;
struct SpriteData {
    int16_t x, y;
    SDL_Color color;
    int sprite_index;
    std::string sprite_name;
    float rotation;
    int layer;
};
struct RenderBuffer {    
    int sprite_count = 0;
    SpriteData *sprite_data_buffer;
};
static const size_t RENDER_BUFFER_MAX = 256;

RenderBuffer render_buffer;

void load_render_data() {
    Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	Resources::font_load("gameover", "pixeltype.ttf", 85);
    sprite_sheets.reserve(8);
    SpriteSheet the_sheet;
	Resources::sprite_sheet_load("shooter_spritesheet.data", the_sheet);
    sprite_sheets.push_back(the_sheet);
    render_buffer.sprite_data_buffer = new SpriteData[RENDER_BUFFER_MAX];
}

// template<class T>
// void ExportSpriteData(const T& go, sprite_data_t& spr)
// {
//     // write out their Position & Sprite data into destination buffer that will be rendered later on.
//     //
//     // Using a smaller global scale "zooms out" the rendering, so to speak.
//     float globalScale = 0.05f;
//     spr.posX = go.pos.x * globalScale;
//     spr.posY = go.pos.y * globalScale;
//     spr.scale = go.sprite.scale * globalScale;
//     spr.colR = go.sprite.colorR;
//     spr.colG = go.sprite.colorG;
//     spr.colB = go.sprite.colorB;
//     spr.sprite = (float)go.sprite.spriteIndex;
// }

template<typename T>
void export_sprite_data(const T &entity_data, const int i, SpriteData &spr) {
    // handle camera, zoom and stuff here

    // float globalScale = 0.05f;
    // spr.x = go.pos.x * globalScale;
    // spr.y = go.pos.y * globalScale;
    // spr.scale = go.sprite.scale * globalScale;

    // spr.x = entity_data.position[i].x - camera.x;
    // spr.x = entity_data.position[i].y - camera.y;

    spr.x = (int16_t)entity_data.position[i].x;
    spr.y = (int16_t)entity_data.position[i].y;
    spr.sprite_index = entity_data.sprite[i].sprite_sheet_index;
    spr.sprite_name = entity_data.sprite[i].sprite_name;
    spr.rotation = entity_data.sprite[i].rotation;
    spr.layer = entity_data.sprite[i].layer;
}

bool sprite_data_sorter(SpriteData const& lhs, SpriteData const& rhs) {
    return lhs.layer < rhs.layer;
}

void export_render_info() {
    render_buffer.sprite_count = 0;
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    for(int i = 0; i < players.length; i++) {
        Direction &d = players.direction[i];
        players.sprite[i].rotation = d.angle + 90;
        export_sprite_data(players, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data(players.position[i], 0, "player_1", d.angle + 90, sprite_data_buffer[sprite_count++]);
        // draw_spritesheet_name_centered_rotated(the_sheet, "player_1", (int)p.x, (int)p.y, d.angle + 90);
        // SDL_Color c = Colors::white;
        // draw_g_circe_color((int16_t)p.x, (int16_t)p.y, 4, c);
    }

    // SDL_Color projectile_color = { 0, 255, 0, 255 };
    for(int i = 0; i < projectiles.length; ++i) {
        export_sprite_data(projectiles, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data(projectiles.position[i], 0, "bullet_2", 0, sprite_data_buffer[sprite_count++]);
        // export_sprite_data(projectiles.position[i], projectile_color, (int16_t)projectiles.radius, sprite_data_buffer[sprite_count++]);
		// Position &p = projectiles.position[i];
		// SDL_Color c = { 0, 255, 0, 255 };
		// draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)projectiles.radius, c);
	}

    // SDL_Color target_color = { 255, 0, 0, 255 };
    for(int i = 0; i < targets.length; ++i) {
        export_sprite_data(targets, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data(targets.position[i], 0, "enemy_1", 0, sprite_data_buffer[sprite_count++]);
        // export_sprite_data(targets.position[i], target_color, (int16_t)targets.radius, sprite_data_buffer[sprite_count++]);
		// Position &p = targets.position[i];
		// SDL_Color c = { 255, 0, 0, 255 };
		// draw_g_circe_color((int16_t)p.x, (int16_t)p.y, (int16_t)targets.radius, c);
	}

    // ASSERT_WITH_MSG(render_buffer.sprite_count < RENDER_BUFFER_MAX, "More sprites exported than buffer can hold");
}

void render_buffer_sort() {
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;
    std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count, sprite_data_sorter);
}

void load_arch() {
    Engine::set_base_data_folder("data");
	
    load_render_data();

    world_bounds = { 0, 0, (int)gw, (int)gh };

    players.allocate(2);
    projectiles.allocate(128);
    targets.allocate(128);
    effects.allocate(128);
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
    
    export_render_info();
    render_buffer_sort();

    FrameLog::log("Players: " + std::to_string(players.length));
    FrameLog::log("Projectiles: " + std::to_string(projectiles.length));
    FrameLog::log("Targets: " + std::to_string(targets.length));
}

void draw_buffer(SpriteData *spr, int length) {
    for(int i = 0; i < length; i++) {
        draw_spritesheet_name_centered_rotated(sprite_sheets[spr[i].sprite_index], spr[i].sprite_name, spr[i].x, spr[i].y, spr[i].rotation);
        // if(spr[i].type == SpriteData::Circle) {
        //     draw_g_circe_color(spr[i].x, spr[i].y, spr[i].radius, spr[i].color);
        // } else if(spr[i].type == SpriteData::Sprite) {
        //     draw_spritesheet_name_centered_rotated(sprite_sheets[spr[i].sprite_index], spr[i].sprite_name, spr[i].x, spr[i].y, spr[i].rotation);
        // }
    }
}

void render_arch() {
    // draw_g_circe_RGBA(gw, 0, 10, 0, 0, 255, 255);
    draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
    FrameLog::render(5, 5);
}

#endif