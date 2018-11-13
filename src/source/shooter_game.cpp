#include "shooter_game.h"

#include <unordered_set>

#include "framework.h"
#include "debug.h"
#include "rendering.h"
#include "entities.h"
#include "systems.h"
#include "particles.h"
#include "event_queue.h"

static ECS::EntityManager entity_manager;
static Player players;
static Projectile projectiles_player;
static Projectile projectiles_target;
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

static Sound::SoundId test_sound_id;

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

void spawn_projectile(Projectile &entity_data, Vector2 p, Vector2 v) {
    auto e = entity_manager.create();
    entity_data.create(e);
    auto handle = entity_data.get_handle(e);
    Position pos = { p, p };
    entity_data.position[handle.i] = pos;
    entity_data.velocity[handle.i] = Velocity(v.x, v.y);
    SpriteComponent s = SpriteComponent("shooter", "bullet_2.png");
    entity_data.sprite[handle.i] = s;
    entity_data.damage[handle.i] = { 1, 2.0f };
    entity_data.collision[handle.i] = { 8 };
}

void spawn_effect(const Position p, const Velocity v, const SpriteComponent s, const EffectData ef) {
    auto e = entity_manager.create();
    effects.create(e);
    auto handle = effects.get_handle(e);
    effects.position[handle.i] = p;
    effects.velocity[handle.i] = v;
    effects.sprite[handle.i] = s;
    effects.effect[handle.i] = ef;
}

void spawn_player(Vector2 position) {
    auto e = entity_manager.create();
    players.create(e);
    auto handle = players.get_handle(e);
    players.position[handle.i] = { position };
    
    SpriteComponent s = SpriteComponent("shooter", "player_1.png");
    s.layer = 1;
    players.sprite[handle.i] = s;
    
    PlayerConfiguration pcfg;
    players.config[handle.i] = pcfg;

    players.health[handle.i] = { 10, 10 };

    players.collision[handle.i] = { 8 };

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
    auto handle = targets.get_handle(e);
    targets.position[handle.i] = { position };
    targets.velocity[handle.i] = { 0, 0 };
    targets.health[handle.i] = { 2, 2 };
    targets.collision[handle.i] = { 8 };
    targets.ai[handle.i] = { 100.0f };
    SpriteComponent s = SpriteComponent("shooter", "enemy_1.png");
    s.layer = 1;
    targets.sprite[handle.i] = s;
}

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
        
	    velocity.value.x += direction.value.x * pi.move_y * player_config.move_acceleration * Time::delta_time;
	    velocity.value.y += direction.value.y * pi.move_y * player_config.move_acceleration * Time::delta_time;

        const auto &player_position = players.position[i];

        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            pi.fire_cooldown = players.weapon[i].fire_cooldown;

            auto projectile_pos = player_position;

            // auto fire_dir = Math::direction(Vector2(Input::mousex, Input::mousey), projectile_pos.value);
            // const Vector2 projectile_direction = fire_dir;
            const Vector2 projectile_direction = direction.value;

            // set the projectile position to be gun_barrel_distance infront of the ship
            projectile_pos.value.x += projectile_direction.x * player_config.gun_barrel_distance;
            projectile_pos.value.y += projectile_direction.y * player_config.gun_barrel_distance;        
            auto muzzle_pos = projectile_pos;

            // Accuracy
            const float accuracy = 8; // how far from initial position it can maximaly spawn
            projectile_pos.value.x += RNG::range_f(-accuracy, accuracy) * projectile_direction.y;
            projectile_pos.value.y += RNG::range_f(-accuracy, accuracy) * projectile_direction.x;

            float projectile_speed = players.weapon[i].projectile_speed;
            Vector2 projectile_velocity = Vector2(projectile_direction.x * projectile_speed, projectile_direction.y * projectile_speed);
            queue_projectile(projectiles_player, projectile_pos.value, projectile_velocity);
            spawn_muzzle_flash(muzzle_pos, Vector2(player_config.gun_barrel_distance, player_config.gun_barrel_distance), players.entity[i]);
            
            camera_shake(0.1f);

            camera_displace(projectile_direction * player_config.fire_knockback_camera);

            smoke_emitter.position = muzzle_pos.value;
            Particles::emit(particles, smoke_emitter);

            // Player knockback
            players.position[i].value.x -= projectile_direction.x * player_config.fire_knockback;
            players.position[i].value.y -= projectile_direction.y * player_config.fire_knockback;

            Sound::queue(test_sound_id, 2);
        }
    }
}

void system_collision_resolution(CollisionPairs &collision_pairs) {
    collision_pairs.sort_by_distance();

    // This set will contain all collisions that we have handled
    // Since first in this instance is projectile and the list is sorted by distance
    // we only care about the collision with the shortest distance in this implementation
    std::unordered_set<ECS::EntityId> handled_collisions;
    for(int i = 0; i < collision_pairs.count; ++i) {
        if(handled_collisions.find(collision_pairs[i].first.id) != handled_collisions.end()) {
            continue;
        }
        debug_config.last_collision_point = collision_pairs[i].collision_point;

        handled_collisions.insert(collision_pairs[i].first.id);

        queue_remove_entity(collision_pairs[i].first);

        if(targets.contains(collision_pairs[i].second)) {
            auto &damage = get_damage(projectiles_player, collision_pairs[i].first);
            auto &health = get_health(targets, collision_pairs[i].second);

            // Knockback
            auto &velocity = get_velocity(projectiles_player, collision_pairs[i].first);
            auto &second_pos = get_position(targets, collision_pairs[i].second);
            Vector2 dir = Math::normalize(Vector2(velocity.value.x, velocity.value.y));
            second_pos.value.x += dir.x * damage.force;
            second_pos.value.y += dir.y * damage.force;

            if(is_invulnerable(health)) {
                continue;
            } 

            deal_damage(damage, health);
            
            Engine::pause(0.03f);

            // Emit hit particles
            const auto &pos = get_position(projectiles_player, collision_pairs[i].first);
            float angle = Math::degrees_between_v(pos.last, collision_pairs[i].collision_point);
            hit_emitter.position = collision_pairs[i].collision_point;
            hit_emitter.angle_min = angle - 10.0f;
            hit_emitter.angle_max = angle + 10.0f;
            Particles::emit(particles, hit_emitter);

            if(health.hp <= 0) {
                // play explosion sound / death sound
                // OR DO THIS IN SPAWN EXPLOSION METHOD
                // Sound::queue(test_sound_id, 2);

                camera_shake(0.1f);
                
                // Spawn explosion particles:
                explosion_emitter.position = second_pos.value;
                Particles::emit(particles, explosion_emitter);

                // spawn explision sprite
                spawn_explosion(second_pos.value, 10, 10);
            } else {
                // play hit sound
                // Sound::queue(test_sound_id, 2);
                
                int blink_frames = 29;

                set_invulnerable(health, 29 * Time::delta_time_fixed);

                blink_sprite(targets, collision_pairs[i].second, blink_frames, 5);
            }
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
            players.sprite[i].sprite_name = "player_turn_right.png";
        } else if(players.input[i].move_x < 0) {
            players.sprite[i].sprite_name = "player_turn_left.png";
        } else {
            players.sprite[i].sprite_name = "player_1.png";
        }
    }
}

void remove_destroyed_entities() {
    for(size_t i = 0; i < entities_to_destroy.size(); i++) {
        Engine::logn("destroying: %d", entities_to_destroy[i].id);
        players.remove(entities_to_destroy[i]);
        projectiles_player.remove(entities_to_destroy[i]);
        projectiles_target.remove(entities_to_destroy[i]);
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

    for(int i = 0; i < projectiles_player.length; ++i) {
        export_sprite_data(projectiles_player, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < projectiles_target.length; ++i) {
        export_sprite_data(projectiles_target, i, sprite_data_buffer[sprite_count++]);
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

    static float projectile_speed = 8.0f;
    
    if(Input::key_pressed(SDLK_UP)) {
        projectile_speed++;
        players.weapon[0].projectile_speed = projectile_speed / 0.016667f;
    }

    if(Input::key_pressed(SDLK_l)) {
        players.health[0].hp -= 1;
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
    FrameLog::log("Projectiles player: " + std::to_string(projectiles_player.length));
    FrameLog::log("Projectiles target: " + std::to_string(projectiles_target.length));
    FrameLog::log("Targets: " + std::to_string(targets.length));
    FrameLog::log("Particles: " + std::to_string(particles.length));
    FrameLog::log("FPS: " + std::to_string(Engine::current_fps));
    FrameLog::log("projectile speed: " + std::to_string(players.weapon[0].projectile_speed));
    FrameLog::log("projectile speed (UP to change): " + std::to_string(projectile_speed));
    // FrameLog::log("Target knockback (L to change): " + std::to_string(target_config.knockback_on_hit));
    
    if(!debug_config.enable_render) {
        return;
    }

    debug_config.render_data.clear();

    debug_export_render_data_circles(players);
    debug_export_render_data_circles(projectiles_player);
    debug_export_render_data_lines(projectiles_player);
    debug_export_render_data_circles(projectiles_target);
    debug_export_render_data_lines(projectiles_target);
    debug_export_render_data_circles(targets);
}

void load_resources() {
    render_buffer.sprite_data_buffer = new SpriteData[RENDER_BUFFER_MAX];

	Resources::font_load("gameover", "pixeltype.ttf", 85);
    
	Resources::sprite_sheet_load("shooter", "shooter_sprites.data");
    // Set up a white copy of the sprite sheet
    Resources::sprite_sheet_copy_as_white("shooterwhite", "shooter");

    test_sound_id = Sound::load("test.wav");

    particles = Particles::make(4096);
    players.allocate(1);
    projectiles_player.allocate(128);
    projectiles_target.allocate(256);
    targets.allocate(128);
    effects.allocate(128);
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
    set_last_position(projectiles_player);
    move_forward(projectiles_player);
    set_last_position(projectiles_target);
    move_forward(projectiles_target);
}

void update_shooter() {
    system_player_get_input(players);
    system_player_handle_input();
    system_ai_input(targets, players, projectiles_target);
    movement();
    system_drag(players);
    system_player_ship_animate();

    // Collision between player and target projectiles

    system_collisions(collisions, projectiles_player, targets);
    system_collision_resolution(collisions);

    system_effects(effects, players, targets);
    system_blink_effect(targets);
    system_camera_follow(players, 0, 100.0f);
    system_invulnerability(targets, Time::delta_time);
    system_invulnerability(players, Time::delta_time);
    system_remove_no_health_left(targets);
    system_remove_no_health_left(players);

    remove_out_of_bounds(projectiles_player, world_bounds);
    remove_out_of_bounds(projectiles_target, world_bounds);

    spawn_projectiles(projectiles_player);
    spawn_projectiles(projectiles_target);
    spawn_effects();
    remove_destroyed_entities();

    Particles::update(particles, Time::delta_time);
    
    export_render_info();

    debug();
}

void render_shooter() {
    draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
    Particles::render_circles_filled(particles);
    debug_render();
}

void render_health_bar(int x, int y, int width, int height, float value, float max) {
    const int border_size = 1;
    const int border_size_d = 2;
    float ratio = value / max;
    auto hp_bar_width = (ratio * (float)width) - border_size_d;
    draw_g_rectangle_filled_RGBA(x, y, width, height, 255, 255, 255, 255);
    draw_g_rectangle_filled_RGBA(x + border_size, y + border_size, width - border_size_d, height - 2, 0, 0, 0, 255);
    draw_g_rectangle_filled_RGBA(x + border_size, y + border_size, (int)hp_bar_width, 13, 255, 0, 0, 255);
}

void render_shooter_ui() {
    if(players.length > 0) {
        render_health_bar(10, 10, 100, 15, (float)players.health[0].hp, (float)players.health[0].hp_max);
    }
    draw_text_centered((int)(gw/2), 10, Colors::white, "UI TEXT");
}