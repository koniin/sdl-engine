#include "level\level.h"

#include <unordered_set>

#include "level\framework.h"
#include "level\debug.h"
#include "level\rendering.h"
#include "level\entities.h"
#include "level\systems.h"
#include "level\game_area.h"
#include "level\game_area_controller.h"
#include "level\ui.h"
#include "level\game_data.h"
#include "particles.h"

static GameArea *game_area;
static GameAreaController *game_area_controller;
static CollisionPairs collisions;
static RenderBuffer render_buffer;

enum State { SettingsSelection, Loading, Play, End } level_state;

void level_load() {
	Resources::font_load("gameover", "pixeltype.ttf", 85);
	Resources::sprite_sheet_load("shooter", "shooter_sprites.data");
    Resources::sprite_sheet_load("deserts", "deserts.data");
    
    GameData::load_data();

    collisions.allocate(128);
    render_buffer.init(2048);
    game_area = new GameArea();
    game_area_controller = new GameAreaController(game_area);
    GameEvents::init(128);
    Timing::init(8);

    game_area_controller->sound_map["basic_fire"] = Sound::load("test.wav");
}

void level_init() {
    level_state = SettingsSelection;
    
    renderer_set_clear_color({ 8, 0, 18, 255 });

    ui_prepare_choices();
    // Do this three times and display the options to the player
    for(int i = 0; i < 3; i++) {
        MapSettings settings;
        generate_settings(settings);
        ui_add_settings_choice(settings);
    }

    std::vector<Upgrade> upgrade_choices;
    generate_random_upgrades(upgrade_choices, 3);
    for(auto &upgrade : upgrade_choices) {
        ui_add_upgrade_choice(upgrade);
    }
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

    system_ammo_recharge(game_area->players);

    system_remove_no_health_left(game_area->targets);
    system_remove_no_health_left(game_area->players);
    remove_out_of_bounds(game_area->projectiles_player, game_area->world_bounds);
    remove_out_of_bounds(game_area->projectiles_target, game_area->world_bounds);
    system_remove_completed_effects(game_area->effects);
    system_update_life_time(game_area->projectiles_player);
    system_update_life_time(game_area->projectiles_target);

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

    if(game_area_controller->spawn_boss()) {
        toasts.push_back({ "Boss is here.", Vector2((float)gw / 2, (float)gh / 2) });
        Timing::add_timer(2.0f, clear_toasts);
        arrow.enabled = true;
    }

    if(game_area_controller->game_over() || game_area_controller->game_win()) {
        level_state = End;
        Particles::clear(game_area->particles);
    } 

    export_render_info(render_buffer, game_area);

    debug(render_buffer, game_area);
}

void start_test() {
    level_state = Play;
}

void level_update() {
    Timing::update_timers();
    ui_update(game_area);

    switch(level_state) {
        case SettingsSelection: {
            MapSettings s;
            if(ui_has_settings_selection(s)) {
                // then after loading screen and selection we do this
                generate_level(s, game_area_controller);
                Timing::add_timer(1.0f, start_test);    
                level_state = Loading;
            }
            break;
        }
        case Loading:
            break;
        case Play:
            game_area_input();
            game_area_update();
            break;
        case End: {
            Upgrade u;
            if(game_area->players.length > 0) {
                if(ui_has_upgrades_selection(u)) {
                    GameData::add_upgrade(u);
                    level_clean();
                    level_init();
                    Engine::logn("Selection");
                }
            }
            else {
                game_area_update();
                if(GInput::pressed(GInput::Start)) {
                    level_clean();
                    level_init();
                }
            }
            break; 
        }  
    }
}

/*
void draw_background(GameArea *ga) {
    auto camera = get_camera();
    int x = Math::max_i(0, 0 - (int)camera.x);
    int y = Math::max_i(0, 0 - (int)camera.y);
    int w = Math::min_i(ga->world_bounds.right() - (int)camera.x, x + gw);
    int h = Math::min_i(ga->world_bounds.bottom() - (int)camera.y, y + gh);
    draw_g_rectangle_filled(x, y, w, h, ga->background_color);
} */

void level_render() {
    switch(level_state) {
        case SettingsSelection:
            ui_render_settings_selection();
            break;
        case Loading:
            break;
        case Play:
            // draw_background();
            draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
            break;
        case End:
            if(game_area->players.length > 0) {
                ui_render_upgrade_selection();
                // draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
                // draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "WIN!"); 
            } else {
                draw_buffer(render_buffer.sprite_data_buffer, render_buffer.sprite_count);
                draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "DEAD! - no implementation of restart etc");
            }
            break;
    }

    Particles::render_circles_filled(game_area->particles);
    debug_render();
}

void level_render_ui() {
    switch(level_state) {
        case SettingsSelection:
            break;
        case Loading:
            break;
        case Play:
            if(game_area->players.length > 0) {
                render_health_bar(10, 10, 100, 15, (float)game_area->players.health[0].hp, (float)game_area->players.health[0].hp_max);
            }
            
            ui_render_map_settings(game_area_controller->map_settings);

            arrow.render();
            
            for(auto &t : toasts) {
                draw_text_centered_str((int)t.position.x, (int)t.position.y, Colors::white, t.message);
            }
            break;
        case End:
            draw_text_centered((int)gw / 2, (int)gh - 20, Colors::white, "Press Start to restart level");
            break;
    }
}

void level_unload() {
    delete game_area_controller;
    delete [] game_area->particles.particles;
    delete [] render_buffer.sprite_data_buffer;
    delete game_area;
}