#ifndef GAME_INPUT_WRAPPER_H
#define GAME_INPUT_WRAPPER_H

#include "engine.h"

namespace GInput {
    enum Action {
        Start = 0,
        Cancel = 1,
        Pause = 2,
        Left = 3,
        Right = 4,
        Up = 5,
        Down = 6,
        Fire = 7,
        ACTION_COUNT
    };

    const static SDL_Keycode input_map[ACTION_COUNT] = {
        SDLK_RETURN, SDLK_BACKSPACE, SDLK_p, SDLK_a, SDLK_d, SDLK_w, SDLK_s, SDLK_SPACE
    };

    inline void direction(Vector2 &direction) {
        if(Input::key_down_k(input_map[Action::Left])) {
            direction.x = -1;
        } else if(Input::key_down_k(input_map[Action::Right])) {
            direction.x = 1;
        }

        if(Input::key_down_k(input_map[Action::Up])) {
            direction.y = 1;
        } else if(Input::key_down_k(input_map[Action::Down])) {
            direction.y = -1;
        }
    }

    inline bool down(const Action &action) {
        return Input::key_down_k(input_map[action]);
    }

    inline bool pressed(const Action &action) {
        return Input::key_pressed(input_map[action]);
    }
}

#endif