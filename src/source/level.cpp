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

static GameArea *game_area;
static GameAreaController *game_area_controller;
static CollisionPairs collisions;
static RenderBuffer render_buffer;

enum State { Loading, Play, End } game_state;

void level_load() {
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter", "shooter_sprites.data");
    Resources::sprite_sheet_load("deserts", "deserts.data");
    
    collisions.allocate(128);
    render_buffer.init(1024);
    game_area = new GameArea();
    game_area_controller = new GameAreaController(game_area);
    GameEvents::init(128);
    Timing::init(8);

    game_area_controller->sound_map["player_fire"] = Sound::load("test.wav");
}

void start_test() {
    game_state = Play;
}


void level_init() {
    game_state = Loading;
    
    renderer_set_clear_color({ 8, 0, 18, 255 });

    int seed = 1338;
    int difficulty = 1;
    int level = 1;

    // Do this three times and display the options to the player
    MapSettings settings;
    generate_settings(seed, difficulty, level, settings);
    generate_level(seed, difficulty, level, settings, game_area_controller);
    
    /*
    renderer_set_clear_color({ 8, 0, 18, 255 });
    game_area->load({ 0, 0, (int)gw * 2, (int)gh * 2 });
    
    Vector2 player_position = Vector2(100, 200);
    game_area_controller->spawn_player(player_position);
    camera_lookat(player_position);

    game_area_controller->spawn_target(Vector2(10, 10));
    game_area_controller->spawn_target(Vector2(400, 200));
    game_area_controller->spawn_target(Vector2(350, 200));
*/
    Timing::add_timer(2.0f, start_test);
}

void level_clean() {
    game_area->clear();
    render_buffer.clear();
    GameEvents::clear();
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

void game_area_input() {
        // Input
    system_player_get_input(game_area->players);
    system_player_handle_input(game_area->players, game_area_controller);
    system_ai_input(game_area->targets, game_area->players, game_area->projectiles_target);
}

void game_area_update() {
    // Movement
    move_forward(game_area->players);
    keep_in_bounds(game_area->players, game_area->world_bounds);
    move_forward(game_area->targets);
    keep_in_bounds(game_area->targets, game_area->world_bounds);
    set_last_position(game_area->projectiles_player);
    move_forward(game_area->projectiles_player);
    set_last_position(game_area->projectiles_target);
    move_forward(game_area->projectiles_target);
    system_drag(game_area->players);
    // ----

    // Collisions
    collisions.clear();
    system_collisions(collisions, game_area->projectiles_player, game_area->targets);
    system_collision_resolution(collisions, game_area->projectiles_player, game_area->targets, game_area_controller);

    collisions.clear();
    system_collisions(collisions, game_area->projectiles_target, game_area->players);
    system_collision_resolution(collisions, game_area->projectiles_target, game_area->players, game_area_controller);
    // ---

    system_player_ship_animate(game_area->players);
    system_target_ship_animate(game_area->targets);
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

    if(game_area_controller->game_over() || game_area_controller->game_win()) {
        game_state = End;
    } 

    export_render_info(render_buffer, game_area);

    debug(game_area);
}

void level_update() {
    Timing::update_timers();
    switch(game_state) {
        case Loading:
            if(Input::key_pressed(SDLK_b)) {
                game_state = Play;
            }
            break;
        case Play:
            game_area_input();
            game_area_update();
            break;
        case End:
            game_area_update();
            if(Input::key_pressed(SDLK_b)) {
                level_clean();
                level_init();
            }
            break;   
    }
}

void draw_background() {
    auto camera = get_camera();
    int x = Math::max_i(0, 0 - (int)camera.x);
    int y = Math::max_i(0, 0 - (int)camera.y);
    int w = Math::min_i(game_area->world_bounds.right() - (int)camera.x, x + gw);
    int h = Math::min_i(game_area->world_bounds.bottom() - (int)camera.y, y + gh);
    draw_g_rectangle_filled(x, y, w, h, game_area->background_color);
}

void level_render() {
    switch(game_state) {
        case Loading:
            draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "LOADING!");
            break;
        case Play:
            draw_background();
            draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
            break;
        case End:
            if(game_area->players.length > 0) {
                draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
                draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "WIN!"); 
            } else {
                draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
                draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "DEAD!");
            }
            break;
    }

    
    auto &camera = get_camera();
    Rectangle view;
    view.x = (int)camera.x - (gw / 2);
    view.y = (int)camera.y - (gh / 2);
    view.w = gw;
    view.h = gh;
    
    draw_g_rectangle_RGBA(view.x, view.y, view.w, view.h, 255, 0, 0, 255);

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
    switch(game_state) {
        case Loading:
            break;
        case Play:
            if(game_area->players.length > 0) {
                render_health_bar(10, 10, 100, 15, (float)game_area->players.health[0].hp, (float)game_area->players.health[0].hp_max);
            }
            draw_text_centered((int)(gw/2), 10, Colors::white, "UI TEXT");
            break;
        case End:
            break;
    }
}

void level_unload() {
    delete game_area_controller;
    delete [] game_area->particles.particles;
    delete [] render_buffer.sprite_data_buffer;
    delete game_area;
}