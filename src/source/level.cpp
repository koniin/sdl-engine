#include "level\level.h"

#include <unordered_set>

#include "level\framework.h"
#include "level\debug.h"
#include "level\rendering.h"
#include "level\entities.h"
#include "level\systems.h"
#include "level\game_area.h"
#include "level\game_area_controller.h"
#include "particles.h"

/* 
    TODO: Need to clean this file from game logic

*/

// TODO: ugly hack
Sound::SoundId test_sound_id;

static GameArea *game_area;
static GameAreaController *game_area_controller;
static CollisionPairs collisions;
static RenderBuffer render_buffer;

void level_load() {
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter", "shooter_sprites.data");
    // Set up a white copy of the sprite sheet
    Resources::sprite_sheet_copy_as_white("shooterwhite", "shooter");
    test_sound_id = Sound::load("test.wav");
    
    collisions.allocate(128);
    render_buffer.init();
    game_area = new GameArea();
    game_area_controller = new GameAreaController(game_area);
    GameEvents::init(128);
}

void level_init() {

    // TODO: set loading screen and load something if needed
    // Then goto game run

    renderer_set_clear_color({ 8, 0, 18, 255 });
    game_area->load({ 0, 0, (int)gw * 2, (int)gh * 2 });
    
    Vector2 player_position = Vector2(100, 200);
    game_area_controller->spawn_player(player_position);
    camera_lookat(player_position);

    game_area_controller->spawn_target(Vector2(10, 10));
    game_area_controller->spawn_target(Vector2(400, 200));
    game_area_controller->spawn_target(Vector2(350, 200));
}

void handle_events(std::vector<GEvent*> &events) {
    for(auto e : events) {
        if(e->is<PlayerFireBullet>()) {
            auto ev = e->get<PlayerFireBullet>();
            Engine::logn("player fired bullet -> %d", ev->test);
        } else {
            Engine::logn("got an event but not what we wanted");
        }
    }
}

void movement() {
    move_forward(game_area->players);
    keep_in_bounds(game_area->players, game_area->world_bounds);
    move_forward(game_area->targets);
    keep_in_bounds(game_area->targets, game_area->world_bounds);
    set_last_position(game_area->projectiles_player);
    move_forward(game_area->projectiles_player);
    set_last_position(game_area->projectiles_target);
    move_forward(game_area->projectiles_target);

    system_drag(game_area->players);
}

void level_update() {
    system_player_get_input(game_area->players);
    system_player_handle_input(game_area->players, game_area_controller);
    system_ai_input(game_area->targets, game_area->players, game_area->projectiles_target);

    movement();
    
    system_player_ship_animate(game_area->players);

    collisions.clear();
    system_collisions(collisions, game_area->projectiles_player, game_area->targets);
    system_collision_resolution(collisions, game_area->projectiles_player, game_area->targets, game_area_controller);

    // Collision between player and target projectiles
    collisions.clear();
    system_collisions(collisions, game_area->projectiles_target, game_area->players);
    system_collision_resolution(collisions, game_area->projectiles_target, game_area->players, game_area_controller);

    system_effects(game_area->effects, game_area->players, game_area->targets);
    system_blink_effect(game_area->targets);
    system_blink_effect(game_area->players);
    
    system_camera_follow(game_area->players, 0, 100.0f);
    system_invulnerability(game_area->targets, Time::delta_time);
    system_invulnerability(game_area->players, Time::delta_time);

    system_remove_no_health_left(game_area->targets);
    system_remove_no_health_left(game_area->players);
    remove_out_of_bounds(game_area->projectiles_player, game_area->world_bounds);
    remove_out_of_bounds(game_area->projectiles_target, game_area->world_bounds);
    system_remove_completed_effects(game_area->effects);

    game_area_controller->spawn_projectiles();
    game_area_controller->spawn_effects();
    
    system_remove_deleted(game_area->players);
    system_remove_deleted(game_area->projectiles_player);
    system_remove_deleted(game_area->projectiles_target);
    system_remove_deleted(game_area->targets);
    system_remove_deleted(game_area->effects);
    
    Particles::update(game_area->particles, Time::delta_time);
    
    handle_events(GameEvents::get_queued_events());
    
    GameEvents::clear();

    export_render_info(render_buffer, game_area);

    debug(game_area);
}

void level_render() {
    draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
    Particles::render_circles_filled(game_area->particles);
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

void level_render_ui() {
    if(game_area->players.length > 0) {
        render_health_bar(10, 10, 100, 15, (float)game_area->players.health[0].hp, (float)game_area->players.health[0].hp_max);
    }
    draw_text_centered((int)(gw/2), 10, Colors::white, "UI TEXT");
}

void level_unload() {
    delete game_area_controller;
    delete [] game_area->particles.particles;
    delete [] render_buffer.sprite_data_buffer;
    delete game_area;
}