#ifndef GAME_INPUT_WRAPPER_H
#define GAME_INPUT_WRAPPER_H

#include "engine.h"

namespace GInput {
    enum Action {
        Start,
        Cancel,
        Left,
        Right,
        Up,
        Down,
        Fire
    };

    struct InputMapping {
        SDL_Scancode up;
        SDL_Scancode down;
        SDL_Scancode left;
        SDL_Scancode right;
        SDL_Scancode fire;
        SDL_Scancode shield;
    };

    const static InputMapping input_maps[2] = {
        { SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_SPACE, SDL_SCANCODE_LSHIFT },
        { SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RSHIFT }
    };

    inline void direction(Vector2 &direction) {
        InputMapping key_map = input_maps[0];
        if(Input::key_down(key_map.left)) {
            direction.x = -1;
        } else if(Input::key_down(key_map.right)) {
            direction.x = 1;
        }

        if(Input::key_down(key_map.up)) {
            direction.y = 1;
        } else if(Input::key_down(key_map.down)) {
            direction.y = -1;
        }
    }

    inline bool down(const Action &action) {
        InputMapping key_map = input_maps[0];
        switch(action) {
            case Fire: { return Input::key_down(key_map.fire); }
            case Left: { return Input::key_down(key_map.left); }
            case Right: { return Input::key_down(key_map.right); }
            case Up: { return Input::key_down(key_map.up); }
            case Down: { return Input::key_down(key_map.down); }
            default: {
                ASSERT_WITH_MSG(false, "KEY NOT IMPLEMENTED");
                break;
            }
        }
    }

    // inline bool pressed(const Action &action) {
    //     InputMapping key_map = input_maps[0];
    //     switch(action) {
    //         case Fire: { return Input::key_pressed(key_map.fire); }
    //         case Left: { return Input::key_pressed(key_map.left); }
    //         case Right: { return Input::key_pressed(key_map.right); }
    //         case Up: { return Input::key_pressed(key_map.up); }
    //         case Down: { return Input::key_pressed(key_map.down); }
    //     }
    // }

    // inline bool down(const Action &action) {
        

    //     switch(action) {
    //         case Left: {
    //             return Input::key_down(key_map.left);
    //         }
    //         case Left: {
    //             return Input::key_down(key_map.left);
    //         }
    //     }
    // }

    // inline void pressed(const Action &action) {
    //     switch(action) {
    //         case Left: {
    //             return ;
    //         }
    //     }
    //     return Input::key_pressed(SDLK_SPACE);
    // }

}

#endif