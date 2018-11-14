#ifndef MENU_H
#define MENU_H

#include "engine.h"
#include "renderer.h"

void menu_load() {
}

void menu_update() {
    if(Input::key_pressed(SDLK_SPACE)) {
        Engine::logn("restart shooter mate!");
    }
}

void menu_render() {
    
}

void menu_render_ui() {
    draw_text_centered((int)gw / 2, (int)gh / 2, Colors::white, "GOOD JOB BUDDY!");
    draw_text_centered((int)gw / 2, (int)gh - 30, Colors::white, "Press space to restart");
}

#endif