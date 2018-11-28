#include "rooms.h"
#include "menu.h"
#include "level/level.h"
#include "level/game_data.h"

#include <array>

static Rooms current_room = NO_ROOM;
static Rooms next_room = NO_ROOM;
static Rooms last_room = NO_ROOM;

void room_load_all() {
    menu_load();
    level_load();
}

void room_unload_all() {
    menu_unload();
    level_unload();
}

void room_goto(const Rooms room) {
    next_room = room;
}

void room_go_back() {
    next_room = last_room;
}

void init_room(const Rooms room) {
    switch(room) {
        case MainMenu:    
            break;
        case NewGame:
            break;
        case Game:
            GameData::game_state_new(); // <- should be in NewGame
            level_init();
            break;
        case AfterGame:
            break;
    }
}

void clean_room(const Rooms room) {
    // Camera reset etc

    switch(room) {
        case MainMenu:    
            break;
        case NewGame:
            break;
        case Game:
            level_clean();
            break;
        case AfterGame:
            break;
    }
}

void room_update() {
    if(current_room != next_room) {
        init_room(next_room);
        clean_room(current_room);
        last_room = current_room;
        current_room = next_room;
    }

    switch(current_room) {
        case MainMenu:
            menu_update();
            break;
        case NewGame:
            break;
        case Game:
            level_update();
            break;
        case AfterGame:
            break;
    }
}

void room_render() {
    switch(current_room) {
        case MainMenu:
            menu_render();
            break;
        case NewGame:
        case Game:
            level_render();
            break;
        case AfterGame:
            break;
    }
}

void room_render_ui() {
    switch(current_room) {
        case MainMenu:
            menu_render_ui();
            break;
        case NewGame:
        case Game:
            level_render_ui();
            break;
        case AfterGame:
            break;
    }
}
