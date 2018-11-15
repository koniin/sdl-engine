#include "menu.h"
#include "rooms.h"

void menu_load() {
    Engine::logn("menu_load");
}

void menu_unload() {
    Engine::logn("menu_unload");
}

void menu_update() {
    if(Input::key_pressed(SDLK_SPACE)) {
        room_goto(Rooms::Game);
    }
}

void menu_render() {
    
}

void menu_render_ui() {
    draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "GOOD JOB BUDDY!");
    draw_text_centered((int)gw / 2, (int)gh - 30, Colors::white, "Press space to restart");
}