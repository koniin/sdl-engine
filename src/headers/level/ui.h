#ifndef UI_H
#define UI_H

#include "engine.h"
#include "renderer.h"
#include "game_area.h"
#include "game_data.h"

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

    std::string hp_text = std::to_string((int)value) + "/" + std::to_string((int)max);
    draw_text_centered_str(x + (int)hp_bar_width / 2, y + height / 2 + border_size, Colors::white, hp_text);
}

template<typename T>
struct SelectList {
    int current = 0;
    int selected = -1;
    std::vector<T> choices;
};

static SelectList<MapSettings> map_settings;
static SelectList<Upgrade> upgrades_selection;

template<typename T>
static void select_list_change_selection(SelectList<T> &select_list, int direction) {
    select_list.current += direction;
    select_list.current = Math::clamp_i(select_list.current, 0, select_list.choices.size() - 1);
}

template<typename T> 
static bool select_list_is_selected(const SelectList<T> &select_list, T &item) {
    if(select_list.selected > -1) {
        item = select_list.choices[select_list.selected];
        return true;
    }
    return false;
}

template<typename T>
static void select_list_reset_choices(SelectList<T> &select_list) {
    select_list.current = 0;
    select_list.selected = -1;
    select_list.choices.clear();
}

template<typename T>
void select_list_select(SelectList<T> &select_list) {
    select_list.selected = select_list.current;
}

template<typename T>
void select_list_change(SelectList<T> &select_list) {
    if(GInput::pressed(GInput::Left)) {
        select_list_change_selection(select_list, -1);
    } else if(GInput::pressed(GInput::Right)) {
        select_list_change_selection(select_list, 1);
    } else if(GInput::pressed(GInput::Start)) {
        select_list_select(select_list);
    }
}

void ui_prepare_choices() {
    select_list_reset_choices(map_settings);
    select_list_reset_choices(upgrades_selection);
}

void ui_add_settings_choice(const MapSettings &item) {
    map_settings.choices.push_back(item);
}

void ui_add_upgrade_choice(const Upgrade &item) {
    upgrades_selection.choices.push_back(item);
}

bool ui_has_settings_selection(MapSettings &item) {
    select_list_change(map_settings);
    return select_list_is_selected(map_settings, item);
}

bool ui_has_upgrades_selection(Upgrade &item) {
    select_list_change(upgrades_selection);
    return select_list_is_selected(upgrades_selection, item);
}

void ui_render_settings_selection() {
    int margin = 160;
    int y = 180;
    int x = margin;
    int count = 0;
    for(auto &settings : map_settings.choices) {
        if(count == map_settings.current) {
            draw_g_rectangle_filled_RGBA(x - 50, y - 10, 100, 100, 40, 40, 0, 255);
        } else {
            draw_g_rectangle_filled_RGBA(x - 50, y - 10, 100, 100, 0, 40, 40, 255);
        }
        
        draw_text_centered(x, y, Colors::white, MapStyleNames[settings.style]);
        draw_text_centered(x, y + 10, Colors::white, MapSizeNames[settings.map_size]);
        x += margin;
        count++;
    }
}

void ui_render_upgrade_selection() {
    int margin = 160;
    int y = 180;
    int x = margin;
    int count = 0;
    for(auto &upgrade : upgrades_selection.choices) {
        if(count == upgrades_selection.current) {
            draw_g_rectangle_filled_RGBA(x - 50, y - 10, 100, 100, 80, 40, 10, 255);
        } else {
            draw_g_rectangle_filled_RGBA(x - 50, y - 10, 100, 100, 80, 120, 40, 255);
        }
        
        draw_text_centered(x, y, Colors::white, upgrade.name);
        draw_text_centered(x, y + 10, Colors::white, upgrade.description);
        x += margin;
        count++;
    }
}

void ui_update(GameArea *ga) {
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