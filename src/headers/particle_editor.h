#include "engine.h"
#include "renderer.h"
#include "particles.h"
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include <fstream>

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


struct TextEditBox {
    int x, y;
    std::string text;
    bool is_active = false;
    int w = 50, h = 20;

    std::vector<char> keys = {
        SDLK_PERIOD,
        SDLK_BACKSLASH,
        SDLK_SLASH,
        SDLK_a,
        SDLK_b,
        SDLK_c,
        SDLK_d,
        SDLK_e,
        SDLK_f,
        SDLK_g,
        SDLK_h,
        SDLK_i,
        SDLK_j,
        SDLK_k,
        SDLK_l,
        SDLK_m,
        SDLK_n,
        SDLK_o,
        SDLK_p,
        SDLK_q,
        SDLK_r,
        SDLK_s,
        SDLK_t,
        SDLK_u,
        SDLK_v,
        SDLK_w,
        SDLK_x,
        SDLK_y,
        SDLK_z,
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9
    };

    void input() {
        if(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h
            && Input::mouse_left_down) {
            is_active = true;
        }
        if(is_active && !(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h)) {
            is_active = false;
        }
        
        if(is_active) {
            if(Input::key_pressed(SDLK_BACKSPACE) && text.length() > 0) {
                text.pop_back();
            } else if(Input::key_pressed(SDLK_RETURN) || Input::key_pressed(SDLK_KP_ENTER)) {
                is_active = false;
            } else if(Input::key_pressed(SDLK_MINUS)) {
                text.push_back('\\');
            } else {
                for(auto &key : keys) {
                    if(Input::key_pressed(key)) {
                        text.push_back(key);
                    }
                }
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

struct SwitchButton {
    int x, y;
    std::string text;
    bool is_active = false;
    int w = 40, h = 15;
    std::vector<int> values;
    std::vector<std::string> values_text;
    int index = 0;

    void input() {
        text = values_text[index];
        is_active = false;
        if(Input::mousex > x && Input::mousex < x + w && Input::mousey > y && Input::mousey < y + h
            && Input::mouse_left_down) {
            is_active = true;
            forward();
        }
    }

    void forward() {
        ++index;
        if(index >= (int)values.size()) {
            index = 0;
        }
    }

    int get_value() {
        return values[index];
    }

    void render() {
        if(is_active) {
            draw_g_rectangle_filled_RGBA(x, y, w, h, 255, 125, 152, 255);
        } else {
            draw_g_rectangle_filled_RGBA(x, y, w, h, 94, 48, 135, 255);
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
    
    bool is_float = true;
    float *value_f = nullptr;
    int *value_i = nullptr;

    float min_val, max_val;

    std::string display;

    Slider() {}

    Slider(std::string name, int xi, int yi, float *val, float min, float max) {
        display = name;
        x = xi;
        y = yi;
        value_f = val;
        min_val = min;
        max_val = max;
        
        float v = ((*val - min) * 100) / (max - min);
        value_x = (int)v;
    }

    Slider(std::string name, int xi, int yi, int *val, float min, float max) {
        display = name;
        x = xi;
        y = yi;
        value_i = val;
        min_val = min;
        max_val = max;
        is_float = false;
        float v = ((*val - min) * 100) / (max - min);
        value_x = (int)v;
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
            value_x = Input::mousex - 5 - x;
            value_x = Math::clamp_i(value_x, 0, w - 10);
            std::string val_x = std::to_string(value_x);
            float v = (((float)value_x) * (max_val - min_val) / 100) + min_val;
            
            if(is_float) {
                *value_f = v;
            } else {
                *value_i = (int)v;
            }
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
        if(is_float) {
            ss << std::fixed << std::setprecision(2) << *value_f;
        } else {
            ss << *value_i;
        }
        draw_text_centered(x + w / 2, y + h / 2, Colors::white, ss.str().c_str());
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

Particles::Emitter emitter_main;
Particles::ParticleContainer particles;
std::vector<ParticleValueEdit> editors;
std::vector<Slider> sliders;
SwitchButton render_mode;
TextEditBox path;


void WriteConfig(const Particles::Emitter &emitter) {
    std::ofstream file;
    file.open(path.text);
    file << emitter.position.x << "\n";
    file << emitter.position.y << "\n";

    file << (int)emitter.color_start.r << "\n";
    file << (int)emitter.color_start.g << "\n";
    file << (int)emitter.color_start.b << "\n";
    file << (int)emitter.color_start.a << "\n";

    file << (int)emitter.color_end.r << "\n";
    file << (int)emitter.color_end.g << "\n";
    file << (int)emitter.color_end.b << "\n";
    file << (int)emitter.color_end.a << "\n";

    file << emitter.force.x << "\n";
    file << emitter.force.y << "\n";

    file << emitter.min_particles << "\n";
    file << emitter.max_particles << "\n";

    file << emitter.life_min << "\n";
    file << emitter.life_max << "\n";
    file << emitter.angle_min << "\n";
    file << emitter.angle_max << "\n";
    file << emitter.speed_min << "\n";
    file << emitter.speed_max << "\n";
    file << emitter.size_min << "\n";
    file << emitter.size_max << "\n";
    file << emitter.size_end_min << "\n";
    file << emitter.size_end_max << "\n";
    file.close();
    Engine::logn("Export complete [%s]", path.text.c_str());
}

std::string format_float(float f) {
    if(f == (int)f) {
        return Text::format("%d", (int)f);
    } else {
        return Text::format("%.3ff", f);
    }
}

void Write_C_Config(const Particles::Emitter &emitter) {
    std::ofstream file;
    file.open(path.text + ".c");

    //file << std::setprecision(4);

    file << "emitter.position = Vector2(" << format_float(emitter.position.x);
    file << ", " << format_float(emitter.position.y) << ");\n";

    file << "emitter.color_start = Colors::make(" << (int)emitter.color_start.r << "," 
        << (int)emitter.color_start.g << "," << (int)emitter.color_start.b << ","
        << (int)emitter.color_start.a << ");\n";

    file << "emitter.color_end = Colors::make(" << (int)emitter.color_end.r << "," 
        << (int)emitter.color_end.g << "," << (int)emitter.color_end.b << ","
        << (int)emitter.color_end.a << ");\n";

    file << "emitter.force = Vector2(" << format_float(emitter.force.x) << ", " << format_float(emitter.force.y) << ");\n";

    file << "emitter.min_particles = " << emitter.min_particles << ";\n";
    file << "emitter.max_particles = " << emitter.max_particles << ";\n";
    file << "emitter.life_min = " << format_float(emitter.life_min) << ";\n";
    file << "emitter.life_max = " << format_float(emitter.life_max) << ";\n";
    file << "emitter.angle_min = " << format_float(emitter.angle_min) << ";\n";
    file << "emitter.angle_max = " << format_float(emitter.angle_max) << ";\n";
    file << "emitter.speed_min = " << format_float(emitter.speed_min) << ";\n";
    file << "emitter.speed_max = " << format_float(emitter.speed_max) << ";\n";
    file << "emitter.size_min = " << format_float(emitter.size_min) << ";\n";
    file << "emitter.size_max = " << format_float(emitter.size_max) << ";\n";
    file << "emitter.size_end_min = " << format_float(emitter.size_end_min) << ";\n";
    file << "emitter.size_end_max = " << format_float(emitter.size_end_max) << ";\n";
    file.close();
    Engine::logn("C Export complete [%s.c]", path.text.c_str());
}

void LoadConfig(Particles::Emitter &emitter) {
    std::ifstream file(path.text);
    file >> emitter.position.x;
    file >> emitter.position.y;

    int c_val;
    file >> c_val;
    emitter.color_start.r = (uint8_t)c_val;
    file >> c_val;
    emitter.color_start.g = (uint8_t)c_val;
    file >> c_val;
    emitter.color_start.b = (uint8_t)c_val;
    file >> c_val;
    emitter.color_start.a = (uint8_t)c_val;

    file >> c_val;
    emitter.color_end.r = (uint8_t)c_val;
    file >> c_val;
    emitter.color_end.g = (uint8_t)c_val;
    file >> c_val;
    emitter.color_end.b = (uint8_t)c_val;
    file >> c_val;
    emitter.color_end.a = (uint8_t)c_val;

    file >> emitter.force.x;
    file >> emitter.force.y;

    file >> emitter.min_particles;
    file >> emitter.max_particles;

    file >> emitter.life_min;
    file >> emitter.life_max;
    file >> emitter.angle_min;
    file >> emitter.angle_max;
    file >> emitter.speed_min;
    file >> emitter.speed_max;
    file >> emitter.size_min;
    file >> emitter.size_max;
    file >> emitter.size_end_min;
    file >> emitter.size_end_max;
    file.close();
    Engine::logn("Import complete [%s]", path.text.c_str());
}

void load_particle_editor() {
    SDL_ShowCursor(SDL_ENABLE);

    particles = Particles::make(4000);

    emitter_main.position = Vector2((float)(gw / 2), (float)(gh / 2));
    emitter_main.color_start = Colors::make(255, 0, 0, 255);
    emitter_main.color_end = Colors::make(255, 0, 0, 0);
    emitter_main.force = Vector2(78, 78);
    emitter_main.min_particles = 30;
    emitter_main.max_particles = 50;
    emitter_main.life_min = 0.1f;
    emitter_main.life_max = 0.3f;
    emitter_main.angle_min = 0;
    emitter_main.angle_max = 360;
    emitter_main.speed_min = 60;
    emitter_main.speed_max = 150;
    emitter_main.size_min = 1;
    emitter_main.size_max = 3;
    emitter_main.size_end_min = 0;
    emitter_main.size_end_max = 0;

    Particles::Emitter &emitter = emitter_main;
    emitter.position = Vector2(320, 180);
    emitter.color_start = Colors::white;
    emitter.color_end = Colors::black;
    emitter.force = Vector2(33, 35);
    emitter.min_particles = 46;
    emitter.max_particles = 86;
    emitter.life_min = 0.100f;
    emitter.life_max = 0.300f;
    emitter.angle_min = 0;
    emitter.angle_max = 360;
    emitter.speed_min = 122;
    emitter.speed_max = 158;
    emitter.size_min = 1;
    emitter.size_max = 3;
    emitter.size_end_min = 4.200f;
    emitter.size_end_max = 9;

    {
        Particles::Emitter &emitter = emitter_main;
        emitter.position = Vector2(320, 180);
        emitter.color_start = Colors::make(229, 130, 0,255);
        emitter.color_end = Colors::make(255,255,255,255);
        emitter.force = Vector2(10, 10);
        emitter.min_particles = 8;
        emitter.max_particles = 12;
        emitter.life_min = 0.200f;
        emitter.life_max = 0.400f;
        emitter.angle_min = 0;
        emitter.angle_max = 360;
        emitter.speed_min = 16;
        emitter.speed_max = 24;
        emitter.size_min = 3;
        emitter.size_max = 5;
        emitter.size_end_min = 6.200f;
        emitter.size_end_max = 9;
    }
    
    path.text = "C:\\temp\\temp_g\\particles\\explosion.particle";
    path.w = 300;
    path.x = gw - path.w;
    path.y = gh - 20; 

    render_mode.values = { 0, 1, 2 };
    render_mode.values_text = { "circle", "fill circle", "fill rectangle" };
    render_mode.w = 100;
    render_mode.x = gw - render_mode.w;
    render_mode.y = gh - 36;

    int x = 10;
    int y = gh - 26 * 7;
    int distance = 24;
    editors.push_back(ParticleValueEdit("count:", x, y, &emitter_main.min_particles, &emitter_main.max_particles));
    editors.push_back(ParticleValueEdit("life:", x, y += distance, &emitter_main.life_min, &emitter_main.life_max));
    editors.push_back(ParticleValueEdit("angle:", x, y += distance, &emitter_main.angle_min, &emitter_main.angle_max));
    editors.push_back(ParticleValueEdit("speed:", x, y += distance, &emitter_main.speed_min, &emitter_main.speed_max));
    editors.push_back(ParticleValueEdit("size:", x, y += distance, &emitter_main.size_min, &emitter_main.size_max));
    editors.push_back(ParticleValueEdit("size end:", x, y += distance, &emitter_main.size_end_min, &emitter_main.size_end_max));
    editors.push_back(ParticleValueEdit("force:", x, y += distance, &emitter_main.force.x, &emitter_main.force.y));

    int slider_y = 10;
    int slider_h = 21;
    int slider_x = gw - 120;
    sliders.push_back(Slider("min_particles", slider_x, slider_y, &emitter_main.min_particles, 0, 200));
    sliders.push_back(Slider("max_particles", slider_x, slider_y += slider_h, &emitter_main.max_particles, 0, 200));

    sliders.push_back(Slider("life_min", slider_x, slider_y += slider_h, &emitter_main.life_min, 0, 10.0f));
    sliders.push_back(Slider("life_max", slider_x, slider_y += slider_h, &emitter_main.life_max, 0, 10.0f));
    sliders.push_back(Slider("angle_min", slider_x, slider_y += slider_h, &emitter_main.angle_min, 0, 360.0f));
    sliders.push_back(Slider("angle_max", slider_x, slider_y += slider_h, &emitter_main.angle_max, 0, 360.0f));
    sliders.push_back(Slider("speed_min", slider_x, slider_y += slider_h, &emitter_main.speed_min, 0, 200.0f));
    sliders.push_back(Slider("speed_max", slider_x, slider_y += slider_h, &emitter_main.speed_max, 0, 200.0f));
    sliders.push_back(Slider("size_min", slider_x, slider_y += slider_h, &emitter_main.size_min, 0, 20.0f));
    sliders.push_back(Slider("size_max", slider_x, slider_y += slider_h, &emitter_main.size_max, 0, 20.0f));

    sliders.push_back(Slider("size_end_min", slider_x, slider_y += slider_h, &emitter_main.size_end_min, 0, 60.0f));
    sliders.push_back(Slider("size_end_max", slider_x, slider_y += slider_h, &emitter_main.size_end_max, 0, 60.0f));

    sliders.push_back(Slider("force x", slider_x, slider_y += slider_h, &emitter_main.force.x, 0, 100.0f));
    sliders.push_back(Slider("force y", slider_x, slider_y += slider_h, &emitter_main.force.y, 0, 100.0f));
}

void update_particle_editor() {
    Particles::update(particles, Time::delta_time);

    FrameLog::log("FPS: " + std::to_string(Engine::current_fps));
    FrameLog::log("Particles: " + std::to_string(particles.length));
    FrameLog::log("Press 'e' to emit");
    FrameLog::log("Press 'w' to write");
    FrameLog::log("Press 'l' to load");
    FrameLog::log("Press '-' to insert '\\' in path ;)");

    path.input();
    if(path.is_active) {
        return;
    }

    if(Input::key_pressed(SDLK_e)) {
        Particles::emit(particles, emitter_main);
    }

    if(Input::key_pressed(SDLK_w)) {
        WriteConfig(emitter_main);
        Write_C_Config(emitter_main);
    }
    
    if(Input::key_pressed(SDLK_l)) {
        LoadConfig(emitter_main);
    }

    for(auto &slider : sliders)
        slider.input();

    for(auto &editor : editors)
        editor.input();

    render_mode.input();
}

void render_particle_editor() {
    switch(render_mode.get_value()) {
        case 0: 
            Particles::render_circles(particles);
            break;
        case 1: 
            Particles::render_circles_filled(particles);
            break;
        case 2: 
            Particles::render_rectangles_filled(particles);
            break;
    }
    
    for(auto &editor : editors)
        editor.render();

    for(auto &slider : sliders)
        slider.render();

    path.render();

    render_mode.render();
}