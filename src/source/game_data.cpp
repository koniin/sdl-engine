#include "level/game_data.h"

namespace GameData {
    GameState *game_state = nullptr;

    void game_state_new(int seed, Difficulty difficulty) {
        if(game_state != nullptr) {
            delete game_state;
        }
        game_state = new GameState(seed, difficulty);
    }

    GameState *game_state_get() {
        return game_state;
    }

    FireSettings create_fire_settings(const Attack &attack, const MapSettings &settings) {
        float fire_cooldown = Attacks[attack].cooldown;
        float accuracy = Attacks[attack].accuracy;
        float knockback = Attacks[attack].knockback;
        float time_to_live = Attacks[attack].range;
        float projectile_speed = Attacks[attack].projectile_speed;
        int projectile_damage = Attacks[attack].projectile_damage;
        int projectile_radius = Attacks[attack].projectile_radius;
        
        ProjectileData p_data(projectile_damage, projectile_radius, time_to_live);

        // Add things from game state like upgrades to player

        // Add things from mapsettings like negative effects and stuff

        char *sound_name = Attacks[attack].sound_name;
        return FireSettings(fire_cooldown, accuracy, projectile_speed, knockback, sound_name, p_data);
    }
};