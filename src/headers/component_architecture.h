
// Game Feel TODO:
/* ==========================

All gfx can be found in shooter_spritesheet.png
[X] Bullet gfx (big)
[X] Ship gfx
[X] Enemy gfx
[X] Muzzle flash (circular filled white first frame or something or display bullet as circle first frame)
[X] Bullet spread (accuracy)
[X] Hit animation (Blink)
[X] Enemy knockback (3 pixels per frame in the direction of the bullet, would be countered by movement in normal cases)
[X] Screen shake on fire weapon
[X] Screen shake on hit enemy
[X] player knockback on fire weapon (if player is too far back move to start pos for demo)
[X] Sleep on hit an enemy (20ms)
* Leave something behind when something is killed
    - destroy hit enemy
    - respawn after some time at random pos on screen
    - debris at spawn site that doesn't move away - how it looks does not matter now
* BIG random explosion/s on kill (circle that flashes from black/grey to white to disappear for one update each)
* Shells or something fly out on fire weapon (make it a "machine gun")
    - check the clip from dropbox

* BIG random explosions

* Smoke on explosion
* Smoke on fire gun

When solid objects that are unbreakable:
* Impact effect (hit effect, like a little marker on the side we hit)

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

#include <unordered_set>

#include "engine.h"
#include "renderer.h"

#include "framework.h"
#include "debug.h"
#include "rendering.h"
#include "entities.h"
#include "systems.h"

template<typename T>
void blink_sprite(T &entity_data, ECS::Entity e, int frames, int interval) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);

    if(entity_data.blink[handle.i].frame_counter > 0)
        return;

    BlinkEffect b;
    b.frames_to_live = frames;
    b.interval = interval;
    b.original_sheet = entity_data.sprite[handle.i].sprite_sheet_index;
    // We assume the next sheet is the white version
    b.white_sheet = entity_data.sprite[handle.i].sprite_sheet_index + 1;
    entity_data.sprite[handle.i].sprite_sheet_index = b.white_sheet;
    entity_data.blink[handle.i] = b;
}

// Pixels per frame
constexpr float player_bullet_speed() {
    return 8.0f / 0.016667f;
}

float movement_per_frame(float val) {
    return val / 0.016667f;
}

constexpr float player_move_acceleration() {
    return 10.0f / 0.016667f;
}

struct PlayerConfiguration {
    int16_t radius = 8;
	float rotation_speed = 3.0f; // degrees
	float move_acceleration = player_move_acceleration();
	float drag = 0.04f;
	float fire_cooldown = 0.15f; // s
	float bullet_speed = player_bullet_speed();
    float gun_barrel_distance = 11.0f; // distance from center
    float fire_knockback = 2.0f; // pixels
} player_config;

struct TargetConfiguration {
    float knockback_on_hit = 2.0f;
} target_config;

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
void queue_projectile(Position p, Vector2 velocity) {
    projectile_queue.push_back({ p, {velocity.x, velocity.y} });
}

void spawn_projectile(Position p, Velocity v) {
    auto e = entity_manager.create();
    projectiles.create(e);
    p.last = p.value;
    set_position(projectiles, e, p);
    set_velocity(projectiles, e, v);
    SpriteComponent s = SpriteComponent(0, "bullet_2");
    set_sprite(projectiles, e, s);
}

void spawn_projectiles() {
    for(size_t i = 0; i < projectile_queue.size(); i++) {
        spawn_projectile(projectile_queue[i].position, projectile_queue[i].velocity);
    }
    projectile_queue.clear();
}

void spawn_player() {
    auto e = entity_manager.create();
    players.create(e);
    set_position(players, e, { Vector2(100, 200) });
    SpriteComponent s = SpriteComponent(0, "player_1");
    s.layer = 1;
    set_sprite(players, e, s);
}

void spawn_target(Vector2 position) {
    auto e = entity_manager.create();
    targets.create(e);
    set_position(targets, e, { position });
    set_velocity(targets, e, { 0, 0 });
    SpriteComponent s = SpriteComponent(0, "enemy_1");
    set_sprite(targets, e, s);
}

struct SpawnEffect {
    Position position;
    Velocity velocity;
    SpriteComponent sprite;
    EffectData effect;
};
std::vector<SpawnEffect> effect_queue;
void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent) {
    auto spr = SpriteComponent(0, "bullet_1");
    spr.layer = effects.effect_layer;
    auto effect = EffectData(2);
    effect.follow = parent;
    effect.local_position = local_position;
    effect.has_target = true;
    effect_queue.push_back({ p, Velocity(), SpriteComponent(0, "bullet_1"), effect });
}

void spawn_effects() {
    for(size_t i = 0; i < effect_queue.size(); i++) {
        auto e = entity_manager.create();
        effects.create(e);
        set_position(effects, e, effect_queue[i].position);
        set_velocity(effects, e, effect_queue[i].velocity);
        set_sprite(effects, e, effect_queue[i].sprite);
        
        auto handle = effects.get_handle(e);
        effects.effect[handle.i] = effect_queue[i].effect;
    }
    effect_queue.clear();
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
        direction.value.x = cos(rotation);
        direction.value.y = sin(rotation);
        
	    velocity.value.x += direction.value.x * pi.move_y * player_config.move_acceleration * Time::deltaTime;
	    velocity.value.y += direction.value.y * pi.move_y * player_config.move_acceleration * Time::deltaTime;

        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            pi.fire_cooldown = player_config.fire_cooldown;

            auto projectile_pos = players.position[i];
            // set the projectile position to be gun_barrel_distance infront of the ship
            projectile_pos.value.x += direction.value.x * player_config.gun_barrel_distance;
            projectile_pos.value.y += direction.value.y * player_config.gun_barrel_distance;        
            auto muzzle_pos = projectile_pos;

            // Accuracy
            const float accuracy = 8; // how far from initial position it can maximaly spawn
            projectile_pos.value.x += RNG::range_f(-accuracy, accuracy) * direction.value.y;
            projectile_pos.value.y += RNG::range_f(-accuracy, accuracy) * direction.value.x;

            Vector2 bullet_velocity = Vector2(direction.value.x * player_config.bullet_speed, direction.value.y * player_config.bullet_speed);
            queue_projectile(projectile_pos, bullet_velocity);
            spawn_muzzle_flash(muzzle_pos, Vector2(player_config.gun_barrel_distance, player_config.gun_barrel_distance), players.entity[i]);
            
            camera_shake(0.1f);

            // Player knockback
            players.position[i].value.x -= direction.value.x * player_config.fire_knockback;
            players.position[i].value.y -= direction.value.y * player_config.fire_knockback;
        }
    }
}

void system_player_drag() {
    for(int i = 0; i < players.length; i++) {
        Velocity &velocity = players.velocity[i];
	    velocity.value.x = velocity.value.x - velocity.value.x * player_config.drag;
	    velocity.value.y = velocity.value.y - velocity.value.y * player_config.drag;
    }
}

void system_move() {
    move_forward(players);
    keep_in_bounds(players, world_bounds);
    move_forward(targets);
    keep_in_bounds(targets, world_bounds);
    for(int i = 0; i < projectiles.length; ++i) {
        projectiles.position[i].last = projectiles.position[i].value;
    }
    move_forward(projectiles);
    remove_out_of_bounds(projectiles, world_bounds);
    system_player_drag();
}

struct CollisionPair {
    ECS::Entity first;
    ECS::Entity second;
    float distance;
    Vector2 collision_point;

    bool operator<( const CollisionPair& rhs ) const { 
        return distance < rhs.distance; 
    }
};

struct CollisionPairs {
    std::vector<CollisionPair> collisions;
    int count = 0;

    inline CollisionPair operator [](size_t i) const { return collisions[i]; }
    inline CollisionPair & operator [](size_t i) { return collisions[i]; }

    void allocate(size_t size) {
        collisions.reserve(size);
    }

    void sort_by_distance() {
        std::sort(collisions.begin(), collisions.end());
    }

    void push(ECS::Entity first, ECS::Entity second, float distance, Vector2 collision_point) {
        collisions.push_back({ first, second, distance, collision_point });
        count++;
    }

    void clear() {
        count = 0;
        collisions.clear();
    }
};

void system_collisions(CollisionPairs &collision_pairs) {
    for(int i = 0; i < projectiles.length; ++i) {
        const Vector2 &p_pos = projectiles.position[i].value;
        const float projectile_radius = (float)projectiles.radius;
        
        for(int j = 0; j < targets.length; ++j) {
            const Vector2 &t_pos = targets.position[j].value;
            const float t_radius = (float)targets.radius;
            const Vector2 &p_last = projectiles.position[i].last;

            // Distance from projectiles last position and targets new position
            // should get the closest target in projectiles path
            float dist = Math::distance_v(p_last, t_pos);
            if(Math::intersect_circles(p_pos.x, p_pos.y, projectile_radius, 
                    t_pos.x, t_pos.y, t_radius)) {
                // Collision point is the point on the target circle 
                // that is on the edge in the direction of the projectiles 
                // reverse velocity
                Engine::logn("circle intersect");
                Vector2 collision_point = t_pos + (t_radius * -projectiles.velocity[i].value.normal());
                collision_pairs.push(projectiles.entity[i], targets.entity[j], dist, collision_point);
                continue;
            }
            
            Vector2 entry_point;
            int result = Intersects::line_circle_entry(p_last, p_pos, t_pos, t_radius, entry_point);
            if(result == 1 || result == 2) {
                Vector2 collision_point = t_pos + (t_radius * -projectiles.velocity[i].value.normal());
                collision_pairs.push(projectiles.entity[i], targets.entity[j], dist, collision_point);
                Engine::logn("line intersect");
            }
        }
    }
    
    // Collision resolution
    collision_pairs.sort_by_distance();
    std::unordered_set<ECS::EntityId> handled_collisions;
    for(int i = 0; i < collision_pairs.count; ++i) {
        if(handled_collisions.find(collision_pairs[i].first.id) != handled_collisions.end()) {
            continue;
        }

        debug_config.last_collision_point = collision_pairs[i].collision_point;

        handled_collisions.insert(collision_pairs[i].first.id);

        queue_remove_entity(collision_pairs[i].first);

        if(targets.contains(collision_pairs[i].second)) {
            blink_sprite(targets, collision_pairs[i].second, 29, 5);

            // Knockback
            auto &velocity = get_velocity(projectiles, collision_pairs[i].first);
            auto &second_pos = get_position(targets, collision_pairs[i].second);
            Vector2 dir = Math::normalize(Vector2(velocity.value.x, velocity.value.y));
            second_pos.value.x += dir.x * target_config.knockback_on_hit;
            second_pos.value.y += dir.y * target_config.knockback_on_hit;
            
            camera_shake(0.1f);

            Engine::pause(0.03f);
        }
    }
    collision_pairs.clear();
}

void system_effects() {
    for(int i = 0; i < effects.length; ++i) {
        if(players.contains(effects.effect[i].follow)) {
            auto handle = players.get_handle(effects.effect[i].follow);
            Position pos = players.position[handle.i];
            const Direction &direction = players.direction[handle.i];
            pos.value.x += direction.value.x * effects.effect[i].local_position.x;
            pos.value.y += direction.value.y * effects.effect[i].local_position.y;
            effects.position[i] = pos;
        }
        if(targets.contains(effects.effect[i].follow)) {
            Engine::logn("following a target - not implemented");
        }
    }

    for(int i = 0; i < effects.length; ++i) {
        effects.effect[i].frame_counter++;
        if(effects.effect[i].frame_counter > effects.effect[i].frames_to_live) {
            queue_remove_entity(effects.entity[i]);
        }
    }
}

void remove_destroyed_entities() {
    for(size_t i = 0; i < entities_to_destroy.size(); i++) {
        Engine::logn("destroying: %d", entities_to_destroy[i].id);
        players.remove(entities_to_destroy[i]);
        projectiles.remove(entities_to_destroy[i]);
        targets.remove(entities_to_destroy[i]);
        effects.remove(entities_to_destroy[i]);

        entity_manager.destroy(entities_to_destroy[i]);
    }
    entities_to_destroy.clear();
}

static CollisionPairs collisions;

static std::vector<SpriteSheet> sprite_sheets;

RenderBuffer render_buffer;

void load_render_data() {
    render_buffer.sprite_data_buffer = new SpriteData[RENDER_BUFFER_MAX];

    Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
	Resources::font_load("gameover", "pixeltype.ttf", 85);
    sprite_sheets.reserve(8);
    SpriteSheet the_sheet;
	Resources::sprite_sheet_load("shooter_spritesheet.data", the_sheet);
    sprite_sheets.push_back(the_sheet);

    // Set up a white copy of the sprite sheet
    Resources::sprite_load_white("shooterwhite", the_sheet.sprite_sheet_name);
    the_sheet.sprite_sheet_name = "shooterwhite";
    sprite_sheets.push_back(the_sheet);
}

void export_render_info() {
    render_buffer.sprite_count = 0;
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    for(int i = 0; i < players.length; i++) {
        Direction &d = players.direction[i];
        players.sprite[i].rotation = d.angle + 90; // sprite is facing upwards so we need to adjust
        export_sprite_data(players, i, sprite_data_buffer[sprite_count++]);
    }

    for(int i = 0; i < projectiles.length; ++i) {
        export_sprite_data(projectiles, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < targets.length; ++i) {
        export_sprite_data(targets, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < effects.length; ++i) {
        export_sprite_data(effects, i, sprite_data_buffer[sprite_count++]);
	}

    // Sort the render buffer by layer
    std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count);
}

void debug() {
    static float bullet_speed = 8.0f;
    
    if(Input::key_pressed(SDLK_UP)) {
        bullet_speed++;
        player_config.bullet_speed = movement_per_frame(bullet_speed);
    }

    if(Input::key_pressed(SDLK_l)) {
        target_config.knockback_on_hit = target_config.knockback_on_hit > 0 ? 0 : 2.0f;
    }

    if(Input::key_pressed(SDLK_F8)) {
        debug_config.enable_render = !debug_config.enable_render;
    }

    FrameLog::log("Press F8 to toggle debug render");
    FrameLog::log("Players: " + std::to_string(players.length));
    FrameLog::log("Projectiles: " + std::to_string(projectiles.length));
    FrameLog::log("Targets: " + std::to_string(targets.length));
    FrameLog::log("FPS: " + std::to_string(Engine::current_fps));
    FrameLog::log("Bullet speed: " + std::to_string(player_config.bullet_speed));
    FrameLog::log("Bullet speed (UP to change): " + std::to_string(bullet_speed));
    FrameLog::log("Target knockback (L to change): " + std::to_string(target_config.knockback_on_hit));

    if(!debug_config.enable_render) {
        return;
    }

    debug_config.render_data.clear();

    for(int i = 0; i < players.length; i++) {
        DebugRenderData d;
        d.x = (int16_t)players.position[i].value.x;
        d.y = (int16_t)players.position[i].value.y;
        d.type = DebugRenderData::Circle;
        d.radius = (int16_t)player_config.radius;
        debug_config.render_data.push_back(d);
    }

    for(int i = 0; i < projectiles.length; ++i) {
        DebugRenderData d;
        d.x = (int16_t)projectiles.position[i].value.x;
        d.y = (int16_t)projectiles.position[i].value.y;
        d.type = DebugRenderData::Circle;
        d.radius = (int16_t)projectiles.radius;
        debug_config.render_data.push_back(d);
        
        d.type = DebugRenderData::Line;
        d.x2 = (int16_t)projectiles.position[i].last.x;
        d.y2 = (int16_t)projectiles.position[i].last.y;
        debug_config.render_data.push_back(d);
	}

    for(int i = 0; i < targets.length; ++i) {
        DebugRenderData d;
        d.x = (int16_t)targets.position[i].value.x;
        d.y = (int16_t)targets.position[i].value.y;
        d.type = DebugRenderData::Circle;
        d.radius = (int16_t)targets.radius;
        debug_config.render_data.push_back(d);
	}
}

void load_arch() {
    Engine::set_base_data_folder("data");
    FrameLog::enable_at(5, 5);
    
    renderer_set_clear_color({ 8, 0, 18, 255 });

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
    spawn_target(Vector2(400, 200));
    spawn_target(Vector2(350, 200));
}

void update_arch() {
    system_player_get_input(players);
    system_player_handle_input();
    system_move();
    system_collisions(collisions);
    system_effects();
    system_blink_effect(targets);

    spawn_projectiles();
    spawn_effects();
    remove_destroyed_entities();

    export_render_info();

    debug();
}

void render_arch() {
    // draw_g_circe_RGBA(gw, 0, 10, 0, 0, 255, 255);
    draw_buffer(sprite_sheets, render_buffer.sprite_data_buffer, render_buffer.sprite_count);
    
    debug_render();
}

void render_ui() {
    draw_text_centered((int)(gw/2), 10, Colors::white, "UI TEXT");
}

#endif