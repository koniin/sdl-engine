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
            draw_g_circe_RGBA(d.x, d.y, d.radius, 255, 0, 0, 255);
        } else if(d.type == DebugRenderData::Line) {
            draw_g_line_RGBA(d.x, d.y, d.x2, d.y2, 0, 255, 0, 255);
        }
    }

    Point p = debug_config.last_collision_point.to_point();
    draw_g_circe_RGBA(p.x, p.y, 2, 255, 0, 0, 255);
}

#endif