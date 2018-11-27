#ifndef UI_H
#define UI_H

#include "engine.h"
#include "renderer.h"
#include "game_area.h"

struct ToastMessage {
    std::string message;
    Vector2 position;
    float ttl = 0;
    float timer = 0;
};
std::vector<ToastMessage> toasts;
void clear_toasts() {
    toasts.clear();
}
void add_toast(std::string message, Vector2 pos, float ttl) {
    toasts.push_back({ message, pos, ttl, 0.0f });
}

struct Arrow {
    bool enabled = false;
    float angle = 0;
    Vector2 position;
    Vector2 center;

    void render() {
        if(enabled) {
            draw_spritesheet_name_centered_rotated(Resources::sprite_sheet_get("shooter"), "arrow", (int)position.x, (int)position.y, angle);
        }
    }

    void update(GameArea *ga) {
        if(!enabled || ga->players.length == 0 || ga->targets.length == 0) {
            return;
        }
        
        auto &camera = get_camera();
        Rectangle view = { (int)camera.x, (int)camera.y, (int)gw, (int)gh };
        
        Vector2 p = ga->players.position[0].value;
        Vector2 b = ga->targets.position[0].value;

        if(view.contains(p.to_point()) && view.contains(b.to_point())) {
            enabled = false;
            return;
        }

        angle = Math::degrees_between_v(p, b);
        if(angle < 0) {
            angle = 360 - (-angle);
        }
        // Atan2 results have 0 degrees point down the positive X axis, while our image is pointed up.
        // Therefore we simply add 90 degrees to the rotation to orient our image
        // If 0 degrees is to the right on your image, you do not need to add 90
        angle = 90 + angle;
        
        center.x = (float)gw / 2;
        center.y = (float)gh / 2;
        auto dir = Math::direction(b, p);
        position = center + dir * 150.0f;
    }

} arrow;

inline void render_health_bar(int x, int y, int width, int height, float value, float max) {
    const int border_size = 1;
    const int border_size_d = 2;
    float ratio = value / max;
    auto hp_bar_width = (ratio * (float)width) - border_size_d;
    draw_g_rectangle_filled_RGBA(x, y, width, height, 255, 255, 255, 255);
    draw_g_rectangle_filled_RGBA(x + border_size, y + border_size, width - border_size_d, height - 2, 0, 0, 0, 255);
    draw_g_rectangle_filled_RGBA(x + border_size, y + border_size, (int)hp_bar_width, 13, 255, 0, 0, 255);
}

void update_ui(GameArea *ga) {
    arrow.update(ga);

    if(!toasts.empty()) {
        for(int i = toasts.size() - 1; i >= 0; i--) {
            toasts[i].timer += Time::delta_time;
            if(toasts[i].timer >= toasts[i].ttl) {
                toasts.erase(toasts.begin() + i); 
            }
        }
    }
}

#endif