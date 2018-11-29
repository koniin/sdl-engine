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

    FireSettings trigger_projectile_fire(const Attack &attack, const MapSettings &settings, 
        float angle, Vector2 pos, std::vector<ProjectileSpawn> &projectiles_queue) 
    {
        Attack_t t_attack = Attacks[attack];

        // run modifiers
        // foreach(var modifier in attack_modifiers) {
        //      modify(modifier, t_attack);
        //      modifier.modify(t_attack);
        // }

        float fire_cooldown = t_attack.cooldown;
        float accuracy = t_attack.accuracy;
        float knockback = t_attack.knockback;
        float time_to_live = t_attack.range;
        float projectile_speed = t_attack.projectile_speed;
        int projectile_damage = t_attack.projectile_damage;
        int projectile_radius = t_attack.projectile_radius;

        // Calculate final accuracy
        float angle_with_accuracy = angle + RNG::range_f(-accuracy, accuracy);
        ProjectileSpawn p(pos, angle_with_accuracy, projectile_speed, projectile_damage, projectile_radius, time_to_live);
        
        switch(attack) {
            case Basic: {
                    projectiles_queue.push_back(p);
                }
                break;
            default:
                ASSERT_WITH_MSG(false, Text::format("Attack not implemented!: %s", AttackNames[attack]));
                break;
        }
        
        char *sound_name = t_attack.sound_name;
        return FireSettings(fire_cooldown, knockback, sound_name);
    }
};