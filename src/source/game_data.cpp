#include "level/game_data.h"

namespace GameData {
    GameState *game_state = nullptr;

    void game_state_new() {
        if(game_state != nullptr) {
            delete game_state;
        }
        game_state = new GameState();
    }

    GameState *game_state_get() {
        return game_state;
    }
};