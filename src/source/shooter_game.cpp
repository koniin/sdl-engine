#include "shooter_game.h"

#include <unordered_set>

#include "framework.h"
#include "debug.h"
#include "rendering.h"
#include "entities.h"
#include "systems.h"
#include "particles.h"

static TargetConfiguration target_config;
static ECS::EntityManager entity_manager;
static Player players;
static Projectile projectiles;
static Target targets;
static Effect effects;
static Rectangle world_bounds;
static RenderBuffer render_buffer;
static CollisionPairs collisions;
static Particles::ParticleContainer particles;
static Particles::Emitter explosion_emitter;
static Particles::Emitter hit_emitter;
static Particles::Emitter exhaust_emitter;
static Particles::Emitter smoke_emitter;

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

// This is one thing?
//////////////////////////////////
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
    SpriteComponent s = SpriteComponent("shooter", "bullet_2.png");
    set_sprite(projectiles, e, s);
}
void spawn_projectiles() {
    for(size_t i = 0; i < projectile_queue.size(); i++) {
        spawn_projectile(projectile_queue[i].position, projectile_queue[i].velocity);
    }
    projectile_queue.clear();
}
//////////////////////////////////

void spawn_player(Vector2 position) {
    auto e = entity_manager.create();
    players.create(e);
    set_position(players, e, { position });
    SpriteComponent s = SpriteComponent("shooter", "player_1.png");
    s.layer = 1;
    set_sprite(players, e, s);

    PlayerConfiguration pcfg;

    auto handle = players.get_handle(e);
    players.config[handle.i] = pcfg;

    SpriteComponent child_sprite = SpriteComponent("shooter", "bullet_2.png");
    child_sprite.h = child_sprite.h + (child_sprite.h / 2);
    child_sprite.layer = 0;
    auto animation = Animation(0.2f, (float)child_sprite.h, (float)child_sprite.h + 4.0f, easing_sine_in_out);
    players.create_child_sprite(pcfg.exhaust_id, e, 
        position, 
        Vector2(-pcfg.gun_barrel_distance, -pcfg.gun_barrel_distance),
        child_sprite,
        animation);
    
}

void spawn_target(Vector2 position) {
    auto e = entity_manager.create();
    targets.create(e);
    set_position(targets, e, { position });
    set_velocity(targets, e, { 0, 0 });
    SpriteComponent s = SpriteComponent("shooter", "enemy_1.png");
    s.layer = 1;
    set_sprite(targets, e, s);
}

// This is one thing?
//////////////////////////////////
struct SpawnEffect {
    Position position;
    Velocity velocity;
    SpriteComponent sprite;
    EffectData effect;
};
std::vector<SpawnEffect> effect_queue;

void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent) {
    auto spr = SpriteComponent("shooter", "bullet_1.png");
    spr.layer = effects.effect_layer;
    auto effect = EffectData(2);
    effect.follow = parent;
    effect.local_position = local_position;
    effect.has_target = true;
    effect_queue.push_back({ p, Velocity(), spr, effect });
}
void spawn_explosion(Vector2 position, float offset_x, float offset_y) {
    auto spr = SpriteComponent("shooter", "explosion_1.png");
    spr.layer = 0;
    auto effect = EffectData(4);
    effect.modifier_enabled = true;
    effect.modifier_data_s = "explosion_2.png";
    effect.modifier_frame = 2;
    effect.modifier = sprite_effect;
    Vector2 blast_position = position;
    blast_position.x += RNG::range_f(-offset_x, offset_x);
    blast_position.y += RNG::range_f(-offset_y, offset_y);
    effect_queue.push_back({ { blast_position }, Velocity(), spr, effect });
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

//////////////////////////////////

void system_player_handle_input() {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        Velocity &velocity = players.velocity[i];
        Direction &direction = players.direction[i];
        const PlayerConfiguration &player_config = players.config[i];
        
        // Update rotation based on rotational speed
        direction.angle += pi.move_x * player_config.rotation_speed;
        if(direction.angle > 360.0f) {
            direction.angle = 0;
        } else if(direction.angle < 0.0f) {
            direction.angle = 360.0f;
        }
        float rotation = direction.angle / Math::RAD_TO_DEGREE;
        direction.value.x = cos(rotation);
        direction.value.y = sin(rotation);
        
	    velocity.value.x += direction.value.x * pi.move_y * player_config.move_acceleration * Time::deltaTime;
	    velocity.value.y += direction.value.y * pi.move_y * player_config.move_acceleration * Time::deltaTime;

        const auto &player_position = players.position[i];

        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            pi.fire_cooldown = player_config.fire_cooldown;

            auto projectile_pos = player_position;

            // auto fire_dir = Math::direction(Vector2(Input::mousex, Input::mousey), projectile_pos.value);
            // const Vector2 bullet_direction = fire_dir;
            const Vector2 bullet_direction = direction.value;

            // set the projectile position to be gun_barrel_distance infront of the ship
            projectile_pos.value.x += bullet_direction.x * player_config.gun_barrel_distance;
            projectile_pos.value.y += bullet_direction.y * player_config.gun_barrel_distance;        
            auto muzzle_pos = projectile_pos;

            // Accuracy
            const float accuracy = 8; // how far from initial position it can maximaly spawn
            projectile_pos.value.x += RNG::range_f(-accuracy, accuracy) * bullet_direction.y;
            projectile_pos.value.y += RNG::range_f(-accuracy, accuracy) * bullet_direction.x;

            Vector2 bullet_velocity = Vector2(bullet_direction.x * player_config.bullet_speed, bullet_direction.y * player_config.bullet_speed);
            queue_projectile(projectile_pos, bullet_velocity);
            spawn_muzzle_flash(muzzle_pos, Vector2(player_config.gun_barrel_distance, player_config.gun_barrel_distance), players.entity[i]);
            
            camera_shake(0.1f);

            camera_displace(bullet_direction * player_config.fire_knockback_camera);

            smoke_emitter.position = muzzle_pos.value;
            Particles::emit(particles, smoke_emitter);

            // Player knockback
            players.position[i].value.x -= bullet_direction.x * player_config.fire_knockback;
            players.position[i].value.y -= bullet_direction.y * player_config.fire_knockback;
        }
    }
}

void system_collision_resolution(CollisionPairs &collision_pairs) {
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

            spawn_explosion(second_pos.value, 10, 10);

            /*
            explosion_emitter.position = second_pos.value;
            Particles::emit(particles, explosion_emitter);
            */
            
            const auto &pos = get_position(projectiles, collision_pairs[i].first);
            float angle = Math::degrees_between_v(pos.last, collision_pairs[i].collision_point);
            hit_emitter.position = collision_pairs[i].collision_point;
            hit_emitter.angle_min = angle - 10.0f;
            hit_emitter.angle_max = angle + 10.0f;
            Particles::emit(particles, hit_emitter);
        }
    }
    collision_pairs.clear();
}

void system_player_ship_animate() {
    system_child_sprite_position(players.child_sprites, players);
    system_child_sprite_exhaust(players, players.child_sprites);
    system_animation_ping_pong(players.child_sprites);

    for(int i = 0; i < players.length; i++) {
        if(players.input[i].move_x > 0) {
            players.sprite[i].sprite_name = "enemy_1.png";
        } else if(players.input[i].move_x < 0) {
            players.sprite[i].sprite_name = "enemy_1.png";
        } else {
            players.sprite[i].sprite_name = "player_1.png";
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

void export_render_info() {
    render_buffer.sprite_count = 0;
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    for(int i = 0; i < players.length; i++) {
        Direction &d = players.direction[i];
        players.sprite[i].rotation = d.angle + 90; // sprite is facing upwards so we need to adjust
        export_sprite_data(players, i, sprite_data_buffer[sprite_count++]);
    }

    for(size_t i = 0; i < players.child_sprites.length; ++i) {
        export_sprite_data(players.child_sprites, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data_values(players.child_sprites.position[i], players.child_sprites[i].sprite, i, sprite_data_buffer[sprite_count++]);
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

// Particles requirements
// * emitters - to emit by position
//      - just update emitter position after moving object
// * no runtime allocations - allocate once
// * draw as sprite or as gemoetry
// * Simple API - update and render should handle itself?

#include "particles.h"

void debug() {
    // Particles::Emitter cfg;
	// cfg.position = Vector2((float)(gw / 2), (float)(gh / 2));
	// cfg.color_start = Colors::make(255, 0, 0, 255);
	// cfg.color_end = Colors::make(255, 0, 0, 0);
	// cfg.force = Vector2(78, 500);
	// cfg.min_particles = 21;
	// cfg.max_particles = 39;
	// cfg.life_min = 1;
	// cfg.life_max = 1.8f;
	// cfg.angle_min = 1.48f;
	// cfg.angle_max = 90;
	// cfg.speed_min = 142;
	// cfg.speed_max = 166;
	// cfg.size_min = 1;
	// cfg.size_max = 3;
	// cfg.size_end_min = 0;
	// cfg.size_end_max = 0;

    // Engine::logn("%d", sizeof(Particles::Particle));

    static float bullet_speed = 8.0f;
    
    if(Input::key_pressed(SDLK_UP)) {
        bullet_speed++;
        players.config[0].bullet_speed = bullet_speed / 0.016667f;
    }

    if(Input::key_pressed(SDLK_l)) {
        target_config.knockback_on_hit = target_config.knockback_on_hit > 0 ? 0 : 2.0f;
    }

    if(Input::key_pressed(SDLK_n)) {
        Particles::emit(particles, explosion_emitter);
    }

    if(Input::key_pressed(SDLK_m)) {
        // char *test;
        // test = new char[1048576]; // allocate 1 megabyte
        // // this memory dangles like crazy
    }

    if(Input::key_pressed(SDLK_F8)) {
        debug_config.enable_render = !debug_config.enable_render;
    }

    FrameLog::log("Press F8 to toggle debug render");
    FrameLog::log("Players: " + std::to_string(players.length));
    FrameLog::log("Projectiles: " + std::to_string(projectiles.length));
    FrameLog::log("Targets: " + std::to_string(targets.length));
    FrameLog::log("Particles: " + std::to_string(particles.length));
    FrameLog::log("FPS: " + std::to_string(Engine::current_fps));
    FrameLog::log("Bullet speed: " + std::to_string(players.config[0].bullet_speed));
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
        d.radius = (int16_t)players.config[i].radius;
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

void load_resources() {
    render_buffer.sprite_data_buffer = new SpriteData[RENDER_BUFFER_MAX];

	Resources::font_load("gameover", "pixeltype.ttf", 85);
    
	Resources::sprite_sheet_load("shooter", "shooter_sprites.data");
    // Set up a white copy of the sprite sheet
    Resources::sprite_sheet_copy_as_white("shooterwhite", "shooter");

    particles = Particles::make(4096);
    players.allocate(1);
    projectiles.allocate(128);
    targets.allocate(128);
    effects.allocate(128);
    projectile_queue.reserve(64);
    entities_to_destroy.reserve(64);
    collisions.allocate(128);

    explosion_emitter.position = Vector2(320, 180);
    explosion_emitter.color_start = Colors::make(229,130,0,255);
    explosion_emitter.color_end = Colors::make(255,255,255,255);
    explosion_emitter.force = Vector2(10, 22);
    explosion_emitter.min_particles = 8;
    explosion_emitter.max_particles = 12;
    explosion_emitter.life_min = 0.100f;
    explosion_emitter.life_max = 0.250f;
    explosion_emitter.angle_min = 0;
    explosion_emitter.angle_max = 360;
    explosion_emitter.speed_min = 32;
    explosion_emitter.speed_max = 56;
    explosion_emitter.size_min = 1.600f;
    explosion_emitter.size_max = 5;
    explosion_emitter.size_end_min = 6.200f;
    explosion_emitter.size_end_max = 9;

    hit_emitter.position = Vector2(320, 180);
    hit_emitter.color_start = Colors::make(229,130,0,255);
    hit_emitter.color_end = Colors::make(255,255,255,255);
    hit_emitter.force = Vector2(0, 0);
    hit_emitter.min_particles = 30;
    hit_emitter.max_particles = 42;
    hit_emitter.life_min = 0.100f;
    hit_emitter.life_max = 0.200f;
    hit_emitter.angle_min = 0;
    hit_emitter.angle_max = 32.400f;
    hit_emitter.speed_min = 122;
    hit_emitter.speed_max = 138;
    hit_emitter.size_min = 2.200f;
    hit_emitter.size_max = 2.600f;
    hit_emitter.size_end_min = 0;
    hit_emitter.size_end_max = 0.600f;

    exhaust_emitter.position = Vector2(320, 180);
    exhaust_emitter.color_start = Colors::make(229,130,0,255);
    exhaust_emitter.color_end = Colors::make(255,255,255,255);
    exhaust_emitter.force = Vector2(10, 10);
    exhaust_emitter.min_particles = 26;
    exhaust_emitter.max_particles = 34;
    exhaust_emitter.life_min = 0.100f;
    exhaust_emitter.life_max = 0.200f;
    exhaust_emitter.angle_min = 0;
    exhaust_emitter.angle_max = 10.800f;
    exhaust_emitter.speed_min = 106;
    exhaust_emitter.speed_max = 130;
    exhaust_emitter.size_min = 0.600f;
    exhaust_emitter.size_max = 1.400f;
    exhaust_emitter.size_end_min = 1.800f;
    exhaust_emitter.size_end_max = 3;

    smoke_emitter.position = Vector2(320, 180);
    smoke_emitter.color_start = Colors::make(165, 165, 165, 255);
    smoke_emitter.color_end = Colors::make(0, 0, 0, 255);
    smoke_emitter.force = Vector2(0, 0);
    smoke_emitter.min_particles = 26;
    smoke_emitter.max_particles = 34;
    smoke_emitter.life_min = 0.200f;
    smoke_emitter.life_max = 0.400f;
    smoke_emitter.angle_min = 0;
    smoke_emitter.angle_max = 320.400f;
    smoke_emitter.speed_min = 3;
    smoke_emitter.speed_max = 6;
    smoke_emitter.size_min = 2;
    smoke_emitter.size_max = 4;
    smoke_emitter.size_end_min = 1;
    smoke_emitter.size_end_max = 2;
}

void init_scene() {
    renderer_set_clear_color({ 8, 0, 18, 255 });
    world_bounds = { 0, 0, (int)gw * 2, (int)gh * 2 };

    Vector2 player_position = Vector2(100, 200);
    spawn_player(player_position);
    camera_lookat(player_position);
    spawn_target(Vector2(10, 10));
    spawn_target(Vector2(400, 200));
    spawn_target(Vector2(350, 200));
}

void load_shooter() {
    load_resources();
    init_scene();
}

void movement() {
    move_forward(players);
    keep_in_bounds(players, world_bounds);
    move_forward(targets);
    keep_in_bounds(targets, world_bounds);
    // Set last position of projectile so we can use that in collision handling etc
    for(int i = 0; i < projectiles.length; ++i) {
        projectiles.position[i].last = projectiles.position[i].value;
    }
    move_forward(projectiles);
    remove_out_of_bounds(projectiles, world_bounds);
}

void update_shooter() {
    system_player_get_input(players);
    system_player_handle_input();
    movement();
    system_drag(players);
    system_player_ship_animate();
    system_collisions(collisions, projectiles, targets);
    system_collision_resolution(collisions);
    system_effects(effects, players, targets);
    system_blink_effect(targets);
    system_camera_follow(players, 0, 100.0f);

    spawn_projectiles();
    spawn_effects();
    remove_destroyed_entities();

    Particles::update(particles, Time::deltaTime);
    
    export_render_info();

    debug();
}

void render_shooter() {
    draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
    Particles::render_circles_filled(particles);
    debug_render();
}

void render_shooter_ui() {
    draw_text_centered((int)(gw/2), 10, Colors::white, "UI TEXT");
}