#ifndef SHOOTER_DEBUG_H
#define SHOOTER_DEBUG_H

#include "engine.h"
#include "renderer.h"

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


void debug(Level *level) {
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
        // char *test;
        // test = new char[1048576]; // allocate 1 megabyte
        // // this memory dangles like crazy
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

#endif