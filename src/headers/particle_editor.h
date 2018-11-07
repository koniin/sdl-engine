#include "engine.h"
#include "renderer.h"
#include "particles.h"

struct EditBox {
    int x, y;
    std::string text;
    bool is_active = false;
    int w = 50, h = 20;

    float *connected_value = nullptr;

    void connect(float *f) {
        connected_value = f;
        text = std::to_string(*f);
    }

    void input() {
        if(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h
            && Input::mouse_left_down) {
            is_active = true;
        }
        if(!(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h)) {
            is_active = false;
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
        }

        if(connected_value != nullptr) {
            if(text.length() > 0)
                *connected_value = std::stof(text);
            else
                *connected_value = 0;
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

struct ParticleValueEdit {
    EditBox min;
    EditBox max;

    std::string headline;

    int pos_x, pos_y;

    void init(std::string h, int x, int y) {
        min.x = x + 40;
        min.y = y;

        max.x = min.x + 5 + min.w;
        max.y = y;

        pos_x = x + 20;
        pos_y = y + 10;

        headline = h;
    }
    
    void connect(float *f1, float *f2) {
        min.connect(f1);
        max.connect(f2);
    }

    void input() {
        min.input();
        max.input();
    }

    void render() {
        draw_text_centered_str(pos_x, pos_y, Colors::white, headline);
        min.render();
        max.render();
    }
};

Particles::Emitter cfg;
ParticleValueEdit box;

void load_particle_editor() {
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

    box.init("life: ", 10, gh - 30);
    box.connect(&cfg.life_min, &cfg.life_max);

    SDL_ShowCursor(SDL_ENABLE);
}

void update_particle_editor() {
    Particles::update(Time::deltaTime);

    if(Input::key_pressed(SDLK_e)) {
        Particles::emit(cfg);
    }

    box.input();

    FrameLog::log("Press 'e' to emit");
}

void render_particle_editor() {
    Particles::render();

    box.render();
}