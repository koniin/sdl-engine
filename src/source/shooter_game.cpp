#include "shooter_game.h"

#include <unordered_set>

#include "framework.h"
#include "debug.h"
#include "rendering.h"
#include "entities.h"
#include "systems.h"
#include "particles.h"

static ShooterGame *_g;
static RenderBuffer render_buffer;
static Sound::SoundId test_sound_id;

template<typename T>
void blink_sprite(T &entity_data, ECS::Entity e, float ttl, float interval) {
    ASSERT_WITH_MSG(entity_data.contains(e), "Entity is not alive");
    auto handle = entity_data.get_handle(e);

    if(entity_data.blink[handle.i].timer > 0)
        return;

    BlinkEffect b;
    b.time_to_live = ttl;
    b.interval = interval;
    b.original_sheet = entity_data.sprite[handle.i].sprite_sheet_index;
    // We assume the next sheet is the white version
    b.white_sheet = entity_data.sprite[handle.i].sprite_sheet_index + 1;
    entity_data.sprite[handle.i].sprite_sheet_index = b.white_sheet;
    entity_data.blink[handle.i] = b;
}

void spawn_player(Vector2 position) {
    auto e = _g->entity_manager.create();
    _g->players.create(e, position);
}

void spawn_target(Vector2 position) {
    auto e = _g->entity_manager.create();
    _g->targets.create(e, position);
}

void system_player_handle_input() {
    Player &players = _g->players;
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        Velocity &velocity = players.velocity[i];
        Direction &direction = players.direction[i];
        const PlayerConfiguration &player_config = players.config[i];
        const auto &player_position = players.position[i];
        
        // Update rotation based on rotational speed
        direction.angle += pi.move_x * player_config.rotation_speed * Time::delta_time;
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
            _g->projectiles_player.queue_projectile(projectile_pos.value, projectile_velocity);
            _g->spawn_muzzle_flash(muzzle_pos, Vector2(player_config.gun_barrel_distance, player_config.gun_barrel_distance), players.entity[i]);
            
            camera_shake(0.1f);

            camera_displace(projectile_direction * player_config.fire_knockback_camera);

            _g->smoke_emitter.position = muzzle_pos.value;
            Particles::emit(_g->particles, _g->smoke_emitter);

            // Player knockback
            players.position[i].value.x -= projectile_direction.x * player_config.fire_knockback;
            players.position[i].value.y -= projectile_direction.y * player_config.fire_knockback;

            Sound::queue(test_sound_id, 2);
        }
    }
}

void on_hit(Projectile &projectile, Player &p, const CollisionPair &entities) {

    // Player is hit

}

void on_hit(Projectile &projectile, Target &t, const CollisionPair &entities) {
    // Knockback
    auto &damage = get_damage(projectile, entities.first);
    auto &velocity = get_velocity(projectile, entities.first);
    auto &second_pos = get_position(t, entities.second);
    Vector2 dir = Math::normalize(Vector2(velocity.value.x, velocity.value.y));
    second_pos.value.x += dir.x * damage.force;
    second_pos.value.y += dir.y * damage.force;
}

// Player is dealt damage
void on_deal_damage(Projectile &projectile, Player &p, const CollisionPair &entities) {
    int amount_dealt = deal_damage(projectile, entities.first, p, entities.second);

    auto &health = get_health(p, entities.second);
    if(health.hp <= 0) {
        // Spawn explosion particles:
        auto &second_pos = get_position(p, entities.second);
        _g->explosion_emitter.position = second_pos.value;
        Particles::emit(_g->particles, _g->explosion_emitter);

        // spawn explision sprite
        _g->spawn_explosion(second_pos.value, 10, 10);

        // Trigger some kind of death thing so we know game is over
    } else if(amount_dealt > 0) {
        // play hit sound
        // Sound::queue(test_sound_id, 2);

        camera_shake(0.1f);

        // 29 frames because that is so cool
        float invulnerability_time = 29 * Time::delta_time_fixed;
        set_invulnerable(health, invulnerability_time);
        blink_sprite(p, entities.second, invulnerability_time, 5 * Time::delta_time_fixed);
    } else {
        Engine::logn("CASE NOT IMPLEMENTED -> no damage dealt on player");
        // Do we need to handle this case?
    }
}

void on_deal_damage(Projectile &projectile, Target &t, const CollisionPair &entities) {
    int amount_dealt = deal_damage(projectile, entities.first, t, entities.second);
    
    Engine::pause(0.03f);

    // Emit hit particles
    const auto &pos = get_position(projectile, entities.first);
    float angle = Math::degrees_between_v(pos.last, entities.collision_point);
    _g->hit_emitter.position = entities.collision_point;
    _g->hit_emitter.angle_min = angle - 10.0f;
    _g->hit_emitter.angle_max = angle + 10.0f;
    Particles::emit(_g->particles, _g->hit_emitter);
    
    auto &health = get_health(t, entities.second);
    if(health.hp <= 0) {
        // play explosion sound / death sound
        // OR DO THIS IN SPAWN EXPLOSION METHOD
        // Sound::queue(test_sound_id, 2);

        camera_shake(0.1f);
        
        // Spawn explosion particles:
        auto &second_pos = get_position(t, entities.second);
        _g->explosion_emitter.position = second_pos.value;
        Particles::emit(_g->particles, _g->explosion_emitter);

        // spawn explosion sprite
        _g->spawn_explosion(second_pos.value, 10, 10);
    } else if(amount_dealt > 0) {
        // play hit sound
        // Sound::queue(test_sound_id, 2);
        
        // 29 frames because that is so cool
        float invulnerability_time = 29 * Time::delta_time_fixed;
        set_invulnerable(health, invulnerability_time);
        blink_sprite(t, entities.second, invulnerability_time, 5 * Time::delta_time_fixed);
    } else {
        Engine::logn("CASE NOT IMPLEMENTED -> no damage dealt");
        // Do we need to handle this case?
    }
}

template<typename First, typename Second>
void system_collision_resolution(CollisionPairs &collision_pairs, First &entity_first, Second &entity_second) {
    collision_pairs.sort_by_distance();

    // This set will contain all collisions that we have handled
    // Since first in this instance is projectile and the list is sorted by distance
    // we only care about the collision with the shortest distance in this implementation
    std::unordered_set<ECS::EntityId> handled_collisions;
    for(int i = 0; i < collision_pairs.count; ++i) {
        if(handled_collisions.find(collision_pairs[i].first.id) != handled_collisions.end()) {
            continue;
        }
        handled_collisions.insert(collision_pairs[i].first.id);
        debug_config.last_collision_point = collision_pairs[i].collision_point;

        mark_for_deletion(entity_first, collision_pairs[i].first);
        
        on_hit(entity_first, entity_second, collision_pairs[i]);
        if(is_invulnerable(entity_second, collision_pairs[i].second)) {
            continue;
        }
        on_deal_damage(entity_first, entity_second, collision_pairs[i]);
    }
}

void system_player_ship_animate() {
    Player &players = _g->players;

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

void export_render_info() {
    render_buffer.sprite_count = 0;
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    for(int i = 0; i < _g->players.length; i++) {
        Direction &d = _g->players.direction[i];
        _g->players.sprite[i].rotation = d.angle + 90; // sprite is facing upwards so we need to adjust
        export_sprite_data(_g->players, i, sprite_data_buffer[sprite_count++]);
    }

    for(size_t i = 0; i < _g->players.child_sprites.length; ++i) {
        export_sprite_data(_g->players.child_sprites, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data_values(players.child_sprites.position[i], players.child_sprites[i].sprite, i, sprite_data_buffer[sprite_count++]);
    }

    for(int i = 0; i < _g->projectiles_player.length; ++i) {
        export_sprite_data(_g->projectiles_player, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < _g->projectiles_target.length; ++i) {
        export_sprite_data(_g->projectiles_target, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < _g->targets.length; ++i) {
        export_sprite_data(_g->targets, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < _g->effects.length; ++i) {
        export_sprite_data(_g->effects, i, sprite_data_buffer[sprite_count++]);
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
        _g->players.weapon[0].projectile_speed = projectile_speed / 0.016667f;
    }

    if(Input::key_pressed(SDLK_l)) {
        _g->players.health[0].hp -= 1;
    }

    if(Input::key_pressed(SDLK_n)) {
        Particles::emit(_g->particles, _g->explosion_emitter);
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
    FrameLog::log("Players: " + std::to_string(_g->players.length));
    FrameLog::log("Projectiles player: " + std::to_string(_g->projectiles_player.length));
    FrameLog::log("Projectiles target: " + std::to_string(_g->projectiles_target.length));
    FrameLog::log("Targets: " + std::to_string(_g->targets.length));
    FrameLog::log("Particles: " + std::to_string(_g->particles.length));
    FrameLog::log("FPS: " + std::to_string(Engine::current_fps));
    FrameLog::log("projectile speed: " + std::to_string(_g->players.weapon[0].projectile_speed));
    FrameLog::log("projectile speed (UP to change): " + std::to_string(projectile_speed));
    // FrameLog::log("Target knockback (L to change): " + std::to_string(target_config.knockback_on_hit));
    
    if(!debug_config.enable_render) {
        return;
    }

    debug_config.render_data.clear();

    debug_export_render_data_circles(_g->players);
    debug_export_render_data_circles(_g->projectiles_player);
    debug_export_render_data_lines(_g->projectiles_player);
    debug_export_render_data_circles(_g->projectiles_target);
    debug_export_render_data_lines(_g->projectiles_target);
    debug_export_render_data_circles(_g->targets);
}

void load_resources() {
    render_buffer.sprite_data_buffer = new SpriteData[RENDER_BUFFER_MAX];

	Resources::font_load("gameover", "pixeltype.ttf", 85);
    
	Resources::sprite_sheet_load("shooter", "shooter_sprites.data");
    // Set up a white copy of the sprite sheet
    Resources::sprite_sheet_copy_as_white("shooterwhite", "shooter");

    test_sound_id = Sound::load("test.wav");

    _g = new ShooterGame({ 0, 0, (int)gw * 2, (int)gh * 2 });
}

void init_scene() {
    renderer_set_clear_color({ 8, 0, 18, 255 });
    
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
    move_forward(_g->players);
    keep_in_bounds(_g->players, _g->world_bounds);
    move_forward(_g->targets);
    keep_in_bounds(_g->targets, _g->world_bounds);
    set_last_position(_g->projectiles_player);
    move_forward(_g->projectiles_player);
    set_last_position(_g->projectiles_target);
    move_forward(_g->projectiles_target);
}

void update_shooter() {
    system_player_get_input(_g->players);
    system_player_handle_input();
    system_ai_input(_g->targets, _g->players, _g->projectiles_target);
    movement();
    system_drag(_g->players);
    system_player_ship_animate();

    _g->collisions.clear();
    system_collisions(_g->collisions, _g->projectiles_player, _g->targets);
    system_collision_resolution(_g->collisions, _g->projectiles_player, _g->targets);

    // Collision between player and target projectiles
    _g->collisions.clear();
    system_collisions(_g->collisions, _g->projectiles_target, _g->players);
    system_collision_resolution(_g->collisions, _g->projectiles_target, _g->players);

    system_effects(_g->effects, _g->players, _g->targets);
    system_blink_effect(_g->targets);
    system_blink_effect(_g->players);
    
    system_camera_follow(_g->players, 0, 100.0f);
    system_invulnerability(_g->targets, Time::delta_time);
    system_invulnerability(_g->players, Time::delta_time);

    system_remove_no_health_left(_g->targets);
    system_remove_no_health_left(_g->players);
    remove_out_of_bounds(_g->projectiles_player, _g->world_bounds);
    remove_out_of_bounds(_g->projectiles_target, _g->world_bounds);
    system_remove_completed_effects(_g->effects);

    _g->spawn_projectiles();
    _g->spawn_effects();
    
    _g->remove_deleted_entities();
    
    Particles::update(_g->particles, Time::delta_time);
    
    export_render_info();

    debug();
}

void render_shooter() {
    draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
    Particles::render_circles_filled(_g->particles);
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
    if(_g->players.length > 0) {
        render_health_bar(10, 10, 100, 15, (float)_g->players.health[0].hp, (float)_g->players.health[0].hp_max);
    }
    draw_text_centered((int)(gw/2), 10, Colors::white, "UI TEXT");
}