#include "engine.h"
#include "renderer.h"
#include "particles.h"
#include <iomanip> // setprecision
#include <sstream> // stringstream

struct EditBox {
    int x, y;
    std::string text;
    bool is_active = false;
    int w = 50, h = 20;

    bool float_value = true;
    float *connected_value = nullptr;
    int *connected_value_i = nullptr;

    void connect(float *f) {
        connected_value = f;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << *f;
        text = ss.str();
    }

    void connect(int *i) {
        connected_value_i = i;
        text = std::to_string(*i);
        float_value = false;
    }

    void input() {
        if(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h
            && Input::mouse_left_down) {
            is_active = true;
        }
        if(is_active && !(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h)) {
            is_active = false;
        }

        if(float_value) {
            float current = text.length() > 0 ? std::stof(text) : 0;
            if(*connected_value != current) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << *connected_value;
                text = ss.str();
            }
        } else {
            int current = text.length() > 0 ? std::stoi(text) : 0;
            if(*connected_value_i != current) {
                text = std::to_string(*connected_value_i);
            }
        }

        if(is_active) {
            if(Input::key_pressed(SDLK_BACKSPACE) && text.length() > 0) {
                text.pop_back();
            }
            if(Input::key_pressed(SDLK_RETURN) || Input::key_pressed(SDLK_KP_ENTER)) {
                is_active = false;
            }
            if(Input::key_pressed(SDLK_1)) {
                text.push_back('1');
            }
            if(Input::key_pressed(SDLK_2)) {
                text.push_back('2');
            }
            if(Input::key_pressed(SDLK_3)) {
                text.push_back('3');
            }
            if(Input::key_pressed(SDLK_4)) {
                text.push_back('4');
            }
            if(Input::key_pressed(SDLK_5)) {
                text.push_back('5');
            }
            if(Input::key_pressed(SDLK_6)) {
                text.push_back('6');
            }
            if(Input::key_pressed(SDLK_7)) {
                text.push_back('7');
            }
            if(Input::key_pressed(SDLK_8)) {
                text.push_back('8');
            }
            if(Input::key_pressed(SDLK_9)) {
                text.push_back('9');
            }
            if(Input::key_pressed(SDLK_0)) {
                text.push_back('0');
            }

            if(float_value) {
                if(text.length() > 0) {
                    *connected_value = std::stof(text);
                } else {
                    *connected_value = 0;
                }
            } else {
                if(text.length() > 0)
                    *connected_value_i = std::stoi(text);
                else
                    *connected_value_i = 0;
            }
        }
    }

    void render() {
        if(is_active) {
            draw_g_rectangle_filled_RGBA(x, y, w, h, 255, 125, 152, 255);
        } else {
            draw_g_rectangle_filled_RGBA(x, y, w, h, 150, 125, 255, 255);
        }
        draw_text_centered_str(x + (w / 2), y + (h / 2), Colors::black, text);
    }
};

struct Slider {
    int x, y;
    std::string text;
    bool is_active = false;
    int w = 110, h = 20;
    int value_x = 10;
    int value_width = 10;

    float *value = nullptr;

    float min_val, max_val;

    std::string display;

    Slider() {}

    Slider(std::string name, int xi, int yi, float *val, float min, float max) {
        display = name;
        x = xi;
        y = yi;
        value = val;
        min_val = min;
        max_val = max;
    }

    void input() {
        if(is_active && Input::mouse_left_down) {
            is_active = false;
        }
        else if(Input::mousex >= x + value_x && Input::mousex < x + value_x + value_width && Input::mousey > y && Input::mousey < y + h
            && Input::mouse_left_down) {
            is_active = true;
        }

        if(is_active) {
            value_x = Input::mousex - x;
            value_x = (int)Math::clamp((float)value_x, 0, (float)w - 10);
            std::string val_x = std::to_string(value_x);
            FrameLog::log("value: " + val_x);
            float v = (((float)value_x) * (max_val - min_val) / 100) + min_val;
            FrameLog::log("new value: " + std::to_string(v));

            *value = v;
        }
    }

    void render() {
        draw_g_rectangle_filled_RGBA(x, y, w, h, 51, 85, 107, 255);
        if(is_active) {
            draw_g_rectangle_filled_RGBA(x + value_x, y, value_width, h, 56, 165, 234, 255);
        } else {
            draw_g_rectangle_filled_RGBA(x + value_x, y, value_width, h, 17, 47, 66, 255);
        }
        std::stringstream ss;
        ss << display << ": ";
        ss << std::fixed << std::setprecision(2) << *value;
        draw_text_centered(x + w / 2, y + h / 2, Colors::black, ss.str().c_str());
    }
};

struct ParticleValueEdit {
    EditBox min;
    EditBox max;

    std::string headline;

    int pos_x, pos_y;

    ParticleValueEdit(std::string text, int x, int y, float *a, float *b) {
        init(text, x, y);
        connect(a, b);
    }

    ParticleValueEdit(std::string text, int x, int y, int *a, int *b) {
        init(text, x, y);
        connect(a, b);
    }

    void init(std::string h, int x, int y) {
        min.x = x + 45;
        min.y = y;

        max.x = min.x + 5 + min.w;
        max.y = y;

        pos_x = x + 40;
        pos_y = y + 5;

        headline = h;
    }
    
    void connect(float *f1, float *f2) {
        min.connect(f1);
        max.connect(f2);
    }

    void connect(int *f1, int *f2) {
        min.connect(f1);
        max.connect(f2);
    }

    void input() {
        min.input();
        max.input();
    }

    void render() {
        draw_text_right_str(pos_x, pos_y, Colors::white, headline);
        min.render();
        max.render();
    }
};

Particles::Emitter cfg;
std::vector<ParticleValueEdit> editors;
std::vector<Slider> sliders;

void load_particle_editor() {
    SDL_ShowCursor(SDL_ENABLE);

    cfg.position = Vector2((float)(gw / 2), (float)(gh / 2));
    cfg.color_start = Colors::make(255, 0, 0, 255);
    cfg.color_end = Colors::make(255, 0, 0, 0);
    cfg.force = Vector2(78, 78);
    cfg.min_particles = 30;
    cfg.max_particles = 50;
    cfg.life_min = 0.1f;
    cfg.life_max = 0.3f;
    cfg.angle_min = 0;
    cfg.angle_max = 360;
    cfg.speed_min = 60;
    cfg.speed_max = 150;
    cfg.size_min = 1;
    cfg.size_max = 3;
    cfg.size_end_min = 0;
    cfg.size_end_max = 0;
    
    int x = 10;
    int y = gh - 26 * 7;
    int distance = 24;
    editors.push_back(ParticleValueEdit("count:", x, y, &cfg.min_particles, &cfg.max_particles));
    editors.push_back(ParticleValueEdit("life:", x, y += distance, &cfg.life_min, &cfg.life_max));
    editors.push_back(ParticleValueEdit("angle:", x, y += distance, &cfg.angle_min, &cfg.angle_max));
    editors.push_back(ParticleValueEdit("speed:", x, y += distance, &cfg.speed_min, &cfg.speed_max));
    editors.push_back(ParticleValueEdit("size:", x, y += distance, &cfg.size_min, &cfg.size_max));
    editors.push_back(ParticleValueEdit("size end:", x, y += distance, &cfg.size_end_min, &cfg.size_end_max));
    editors.push_back(ParticleValueEdit("force:", x, y += distance, &cfg.force.x, &cfg.force.y));

    sliders.push_back(Slider("life_min", 100, 10, &cfg.life_min, 0, 10.0f));
    
}

void update_particle_editor() {
    Particles::update(Time::deltaTime);

    if(Input::key_pressed(SDLK_e)) {
        Particles::emit(cfg);
    }

    for(auto &slider : sliders)
        slider.input();

    for(auto &editor : editors)
        editor.input();

    FrameLog::log("Press 'e' to emit");
}

void render_particle_editor() {
    Particles::render();

    for(auto &editor : editors)
        editor.render();

    for(auto &slider : sliders)
        slider.render();
}