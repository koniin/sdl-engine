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

    FireSettings create_fire_settings(const Attack &attack, const MapSettings &settings) {
        float fire_cooldown = Attacks[attack].cooldown;
        float accuracy = Attacks[attack].accuracy;
        float projectile_speed = Attacks[attack].projectile_speed;
        float knockback = Attacks[attack].knockback;
        char *sound_name = Attacks[attack].sound_name;
        ProjectileData p_data;

        return FireSettings(fire_cooldown, accuracy, projectile_speed, knockback, sound_name, p_data);
    }
};