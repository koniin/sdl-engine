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
    system_homing(game_area->projectiles_player, game_area->targets);
    set_last_position(game_area->projectiles_player);
    move_forward(game_area->projectiles_player);
    set_last_position(game_area->projectiles_target);
    move_forward(game_area->projectiles_target);
    system_drag(game_area->players, player_drag() * 0.2f);
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
    system_shield_recharge(game_area->players);

    system_remove_no_health_left(game_area->targets);
    system_remove_no_health_left(game_area->players);
    remove_out_of_bounds(game_area->projectiles_player, game_area->world_bounds);
    remove_out_of_bounds(game_area->projectiles_target, game_area->world_bounds);
    system_remove_completed_effects(game_area->effects);
    system_update_life_time(game_area->projectiles_player);
    system_update_life_time(game_area->projectiles_target);

    system_on_death(game_area->projectiles_player, game_area_controller);

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

static int some_y = 0;
static int some_x = 0;
static double some_r = 0;

void level_update() {
    Timing::update_timers();
    ui_update(game_area);

    if(Input::key_pressed(SDLK_UP)) {
        some_y -= 5;
    }
    if(Input::key_pressed(SDLK_DOWN)) {
        some_y += 5;
    }
    if(Input::key_pressed(SDLK_LEFT)) {
        some_x -= 5;
    }
    if(Input::key_pressed(SDLK_RIGHT)) {
        some_x += 5;
    }

    if(Input::key_pressed(SDLK_g)) {
        some_r -= 5;
    }

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

void calc_lazer(SDL_Rect &lazer_rect, const Vector2 &start, const Vector2 &end, const int &height, const double &rotation) {
    float distance = Math::distance_v(start, end);
    Vector2 difference = end - start;
    lazer_rect.x = start.x + (difference.x / 2) - (distance / 2);
    lazer_rect.y = start.y + (difference.y / 2) - (height / 2);
    lazer_rect.w = distance;
    lazer_rect.h = height;
}

void lazer() {
    
    sprite_sheets = &Resources::get_sprite_sheets();
    auto &sheet = sprite_sheets->at(0);

    // --- Player
    
    auto *p_region = &sheet.sheet_sprites[sheet.sprites_by_name.at("player_1")].region;
    auto p_image = Resources::sprite_get(sheet.sprite_sheet_name)->image;
    
    int player_x = 100 + some_x;
    int player_y = 150 + some_y;
    int player_center_x = player_x + 8;
    int player_center_y = player_y + 8;
    int target_x = 200;
    int target_y = 100;
    int target_center_x = target_x + 8;
    int target_center_y = target_y + 8;
    auto r = Math::rads_between_f(player_center_x, player_center_y, target_center_x, target_center_y) * Math::RAD_TO_DEGREE;
    r += some_r;
    int lazer_height = 8;
    int lazer_half_height = 4;



    SDL_Rect destination_rect;
	destination_rect.x = player_x;
 	destination_rect.y = player_y;
  	destination_rect.w = p_region->w;
  	destination_rect.h = p_region->h;

	SDL_RenderCopyEx(renderer.renderer, p_image, p_region, &destination_rect, r + 90, NULL, SDL_RendererFlip::SDL_FLIP_NONE);

    // ------


    // --- Target
    {
        auto *t_region = &sheet.sheet_sprites[sheet.sprites_by_name.at("enemy_1")].region;
        auto t_image = Resources::sprite_get(sheet.sprite_sheet_name)->image;
        
        SDL_Rect destination_rect;
        destination_rect.x = target_x;
        destination_rect.y = target_y;
        destination_rect.w = t_region->w;
        destination_rect.h = t_region->h;

        SDL_RenderCopyEx(renderer.renderer, t_image, t_region, &destination_rect, 0, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
    }
    // ------


    // Lazer

    auto *l_region = &sheet.sheet_sprites[sheet.sprites_by_name.at("lazer")].region;
    auto l_image = Resources::sprite_get(sheet.sprite_sheet_name)->image;
    
    SDL_Rect area_rect;
    area_rect.x = player_center_x;
 	area_rect.y = player_center_y;
  	area_rect.w = (target_center_x - player_center_x);
  	area_rect.h = (target_center_y - player_center_y);
    SDL_SetRenderDrawColor(renderer.renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer.renderer, &area_rect);

    // Engine::logn("H: %d,  %d", (area_rect.h / 2), (player_y - target_y) / 2);

    
    
    std::string t_log = "y | p-t: " + std::to_string(player_center_y - target_center_y) 
        + ", t-p: " + std::to_string(target_center_y - player_center_y)
        + ", abs: " + std::to_string(Math::abs(player_center_y - target_center_y));
    draw_text_centered(150, gh - 10, Colors::white, t_log.c_str());
    

    float distance = Math::distance_f(player_center_x, player_center_y, target_center_x, target_center_y);
    auto x_distance_to_target = target_center_x - player_center_x;
    auto y_distance_to_target = target_center_y - player_center_y;
    SDL_Rect lazer_rect;
    lazer_rect.x = player_center_x + (x_distance_to_target / 2) - (distance / 2);
    lazer_rect.y = player_center_y + (y_distance_to_target / 2) - lazer_half_height;
    lazer_rect.w = distance;
    lazer_rect.h = 8;
    SDL_RenderDrawRect(renderer.renderer, &lazer_rect);
    SDL_RenderCopyEx(renderer.renderer, l_image, l_region, &lazer_rect, r, NULL, SDL_RendererFlip::SDL_FLIP_NONE);

    /* Shit works
    if(player_center_x < target_center_x) {
        float distance = Math::distance_f(player_center_x, player_center_y, target_center_x, target_center_y);
        SDL_Rect lazer_rect;
        area_rect.w = target_center_x - player_center_x;
        lazer_rect.x = player_center_x + area_rect.w / 2 - distance / 2;
        lazer_rect.y = player_center_y + (area_rect.h / 2) - lazer_half_height;
        lazer_rect.w = distance;
        lazer_rect.h = 8;
        SDL_RenderDrawRect(renderer.renderer, &lazer_rect);
        SDL_RenderCopyEx(renderer.renderer, l_image, l_region, &lazer_rect, r, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
    } else {
        float distance = Math::distance_f(target_center_x, target_center_y, player_center_x, player_center_y);
        area_rect.w = player_center_x - target_center_x;
        area_rect.h = player_center_y - target_center_y;
        SDL_Rect lazer_rect;
        lazer_rect.x = target_center_x + area_rect.w / 2 - distance / 2;
        lazer_rect.y = target_center_y + (area_rect.h / 2) - lazer_half_height;
        lazer_rect.w = distance;
        lazer_rect.h = 8;
        SDL_RenderDrawRect(renderer.renderer, &lazer_rect);
        SDL_RenderCopyEx(renderer.renderer, l_image, l_region, &lazer_rect, r, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
    }*/

    /*  GOOD SHIT
    SDL_Rect lazer_rect;
	lazer_rect.x = player_x;
 	lazer_rect.y = player_center_y - Math::abs(area_rect.h / 2) - lazer_half_height;
  	lazer_rect.w = Math::distance_f(player_x, player_y, target_x, target_y);
  	lazer_rect.h = 8;
    */
    SDL_Rect lazer_rect;
    Vector2 player_pos = Vector2(player_center_x, player_center_y);
    Vector2 target_pos = Vector2(target_center_x, target_center_y);
    calc_lazer(lazer_rect, player_pos, target_pos, 8, r);
    SDL_RenderDrawRect(renderer.renderer, &lazer_rect);
    SDL_RenderCopyEx(renderer.renderer, l_image, l_region, &lazer_rect, r, NULL, SDL_RendererFlip::SDL_FLIP_NONE);

    draw_g_line_RGBA(player_center_x, player_center_y, target_center_x, target_center_y, 0, 255, 0, 255);	
}

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

    lazer();
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