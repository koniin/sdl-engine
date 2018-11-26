#ifndef SHOOTER_DEBUG_H
#define SHOOTER_DEBUG_H

#include "engine.h"
#include "renderer.h"
#include "game_area.h"
#include "rooms.h"
#include "generator.h"
#include "rendering.h"

struct DebugRenderData {
    enum Type { Circle, Line } type;
    int16_t x, y;
    int16_t x2, y2;
    int16_t radius;
};
struct DebugConfiguration {
    bool enable_render = false;
    std::vector<DebugRenderData> render_data;
    Vector2 last_collision_point;
} debug_config;

template<typename T> 
void debug_export_render_data_circles(const T &entity_data) {
    const auto &camera = get_camera();
    for(int i = 0; i < entity_data.length; i++) {
        DebugRenderData d;
        d.x = (int16_t)(entity_data.position[i].value.x - camera.x);
        d.y = (int16_t)(entity_data.position[i].value.y - camera.y);
        d.type = DebugRenderData::Circle;
        d.radius = (int16_t)entity_data.collision[i].radius;
        debug_config.render_data.push_back(d);
    }
}

template<typename T> 
void debug_export_render_data_lines(const T &entity_data) {
    const auto &camera = get_camera();
    for(int i = 0; i < entity_data.length; ++i) {
        DebugRenderData d;
        d.x = (int16_t)(entity_data.position[i].value.x - camera.x);
        d.y = (int16_t)(entity_data.position[i].value.y - camera.y);
        d.type = DebugRenderData::Line;
        d.x2 = (int16_t)(entity_data.position[i].last.x - camera.x);
        d.y2 = (int16_t)(entity_data.position[i].last.y - camera.y);
        debug_config.render_data.push_back(d);
    }
}

void debug(RenderBuffer &render_buffer, GameArea *level) {
    static float projectile_speed = 8.0f;
    
    if(Input::key_pressed(SDLK_UP)) {
        projectile_speed++;
        level->players.weapon[0].projectile_speed = projectile_speed / 0.016667f;
    }

    if(Input::key_pressed(SDLK_l)) {
        level->players.health[0].hp -= 1;
    }

    if(Input::key_pressed(SDLK_n)) {
        Particles::emit(level->particles, level->explosion_emitter);
    }

    if(Input::key_pressed(SDLK_m)) {
        room_goto(Rooms::MainMenu);
        // char *test;
        // test = new char[1048576]; // allocate 1 megabyte
        // // this memory dangles like crazy
    }

    if(Input::key_pressed(SDLK_g)) {
        RDSTable table;
        table.rdsContents = {
            { 
                1.0f, // The chance for this item to drop
                false, // Only drops once per query
                false, // Drops always
                1
            },
            { 
                2.0f, // The chance for this item to drop
                false, // Only drops once per query
                false, // Drops always
                2
            },
            { 
                3.0f, // The chance for this item to drop
                false, // Only drops once per query
                false, // Drops always
                3
            },
            { 
                5.0f, // The chance for this item to drop
                true, // Only drops once per query
                false, // Drops always
                5
            }
        };
        auto r = rds(2, table);
        for(auto &o : r) {
            Engine::logn("in: %d", o.id);
        }
    }

    if(Input::key_pressed(SDLK_F8)) {
        debug_config.enable_render = !debug_config.enable_render;
    }

    FrameLog::log("Press F8 to toggle debug render");
    FrameLog::log("Players: " + std::to_string(level->players.length));
    FrameLog::log("Projectiles player: " + std::to_string(level->projectiles_player.length));
    FrameLog::log("Projectiles target: " + std::to_string(level->projectiles_target.length));
    FrameLog::log("Targets: " + std::to_string(level->targets.length));
    FrameLog::log("Particles: " + std::to_string(level->particles.length));
    FrameLog::log("FPS: " + std::to_string(Engine::current_fps));
    FrameLog::log("projectile speed: " + std::to_string(level->players.weapon[0].projectile_speed));
    FrameLog::log("projectile speed (UP to change): " + std::to_string(projectile_speed));

    FrameLog::log("Render buffer count: " + std::to_string(render_buffer.sprite_count));
    FrameLog::log("Tile count: " + std::to_string(level->tiles.size()));
    //auto &camera = get_camera();
    //FrameLog::log("Camera x: " + std::to_string(camera.x) + ", y: " + std::to_string(camera.y));
    // FrameLog::log("Target knockback (L to change): " + std::to_string(target_config.knockback_on_hit));
    
    if(!debug_config.enable_render) {
        return;
    }

    debug_config.render_data.clear();

    debug_export_render_data_circles(level->players);
    debug_export_render_data_circles(level->projectiles_player);
    debug_export_render_data_lines(level->projectiles_player);
    debug_export_render_data_circles(level->projectiles_target);
    debug_export_render_data_lines(level->projectiles_target);
    debug_export_render_data_circles(level->targets);
}

void debug_render() {
    if(!debug_config.enable_render) {
        return;
    }

    for(auto &d : debug_config.render_data) {
        if(d.type == DebugRenderData::Circle) {
            draw_g_circle_RGBA(d.x, d.y, d.radius, 255, 0, 0, 255);
        } else if(d.type == DebugRenderData::Line) {
            draw_g_line_RGBA(d.x, d.y, d.x2, d.y2, 0, 255, 0, 255);
        }
    }

    Point p = debug_config.last_collision_point.to_point();
    draw_g_circle_RGBA(p.x, p.y, 2, 255, 0, 0, 255);
}

#endif