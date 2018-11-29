#include "level/game_data.h"

static void add_upgrades_to_list(std::vector<Upgrade> &upgrades);

namespace GameData {
    GameState *game_state = nullptr;
    std::vector<Upgrade> upgrades;

    void game_state_new(int seed, Difficulty difficulty) {
        if(game_state != nullptr) {
            delete game_state;
        }
        game_state = new GameState(seed, difficulty);
    }

    GameState *game_state_get() {
        return game_state;
    }

    void load_upgrades() {
        add_upgrades_to_list(upgrades);
    }

    std::vector<Upgrade> &get_upgrades() {
        return upgrades;
    }

    void set_attack(const Attack & attack) {
        game_state->player.attack = attack;
    }

    void add_upgrade(const Upgrade &u) {
        auto &player_state = game_state->player;
        u.apply_player_modifiers(player_state);

        game_state->player_upgrades.push_back(u);

        /*
            { "Phat", "+1 MAX hp" }, 


            { "Muscle Up", "damage increase" },
            { "Spray and pray", "Projectile speed increase" },
            { "Pierced from within", "Projectiles pierce" }
        */
    }

    // Order
    // 1. first we take the data from the attack and use as base
    // 2. apply modifiers to the base
    // 3. apply map settings things to that
    FireSettings trigger_projectile_fire(const Attack &attack, const MapSettings &settings, 
        float angle, Vector2 pos, std::vector<ProjectileSpawn> &projectiles_queue) 
    {
        Attack_t t_attack = Attacks[attack];

        for(auto &upgrade : game_state->player_upgrades) {
            upgrade.apply_projectile_modifiers(t_attack);
        }

        // Apply map modifiers
        
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

static void add_upgrades_to_list(std::vector<Upgrade> &upgrades) {
    {
        Upgrade u = { "Phat", "+1 MAX hp" };
        PlayerModifier m;
        m.max_hp = 1;

        u.player_m.push_back(m);

        upgrades.push_back(u);
    }
    {
        Upgrade u = { "Muscle Up", "damage increase" };
        ProjectileStatModifier m;
        m.projectile_damage = 1;
        u.projectile_m.push_back(m);

        upgrades.push_back(u);
    }
    {
        Upgrade u = { "Spray and pray", "Projectile speed increase" };
        ProjectileStatModifier m;
        m.projectile_speed = per_frame_calculation(2);
        u.projectile_m.push_back(m);

        upgrades.push_back(u);
    }
}