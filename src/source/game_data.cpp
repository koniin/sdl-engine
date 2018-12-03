#include "level/game_data.h"

static void add_upgrades_to_list(std::vector<Upgrade> &upgrades);
static void add_map_modifiers_to_list(std::vector<MapModifier> &modifiers);

namespace GameData {
    GameState *game_state = nullptr;
    std::vector<Upgrade> upgrades;
    std::vector<MapModifier> map_modifiers;

    void game_state_new(int seed, Difficulty difficulty) {
        if(game_state != nullptr) {
            delete game_state;
        }
        game_state = new GameState(seed, difficulty);
    }

    GameState *game_state_get() {
        return game_state;
    }

    void load_data() {
        add_upgrades_to_list(upgrades);
        add_map_modifiers_to_list(map_modifiers);
    }

    std::vector<Upgrade> &get_upgrades() {
        return upgrades;
    }
    
    std::vector<MapModifier> &get_map_modifiers() {
        return map_modifiers;
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
    // 2. apply player upgrade modifiers to the base
    // 3. apply map settings things to that
    // 4. then we fire the projectiles based on those numbers
    FireSettings trigger_projectile_fire(const Attack &attack, const MapSettings &settings, 
        float angle, Vector2 pos, std::vector<ProjectileSpawn> &projectiles_queue) 
    {
        Attack_t t_attack = Attacks[attack];

        for(auto &upgrade : game_state->player_upgrades) {
            upgrade.apply_projectile_modifiers(t_attack);
        }

        settings.apply_player_projectile_modifiers(t_attack);
        
        float fire_cooldown = t_attack.cooldown;
        float accuracy = t_attack.accuracy;
        float knockback = t_attack.knockback;
        float time_to_live = t_attack.range;
        float projectile_speed = t_attack.projectile_speed;
        float projectile_speed_mod = t_attack.projectile_speed_mod;
        int projectile_damage = t_attack.projectile_damage;
        int projectile_radius = t_attack.projectile_radius;
        int pierce_count = t_attack.pierce_count;
        std::vector<float> &angles = t_attack.projectile_angles;

        Engine::logn("pierce: %d", pierce_count);

        for(auto &angle_offset : angles) {
            float final_angle = angle + angle_offset + RNG::range_f(-accuracy, accuracy);
            float final_speed = projectile_speed + RNG::range_f(-projectile_speed_mod, projectile_speed_mod);
            ProjectileSpawn p(pos, final_angle, final_speed, projectile_damage, projectile_radius, time_to_live, pierce_count);
            projectiles_queue.push_back(p);
        }

        // // TODO: Make this into properties of the attack instead
        // //       so it can decide how many projectiles to fire and their properties instead
        // switch(attack) {
        //     case Basic: {
                    
        //         }
        //         break;
        //     default:
        //         ASSERT_WITH_MSG(false, Text::format("Attack not implemented!: %s", AttackNames[attack]));
        //         break;
        // }
        
        char *sound_name = t_attack.sound_name;
        return FireSettings(fire_cooldown, knockback, sound_name);
    }
};

static void add_upgrades_to_list(std::vector<Upgrade> &upgrades) {
    {
        Upgrade u = { "Phat", "+4 MAX hp" };
        PlayerModifier m;
        m.max_hp = 4;

        u.player_m.push_back(m);

        upgrades.push_back(u);
    }
    {
        Upgrade u = { "Muscle Up", "damage increase" };
        ProjectileStatModifier m;
        m.projectile_damage = 2;
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

static void add_map_modifiers_to_list(std::vector<MapModifier> &modifiers) {
    { 
        MapModifier m = { "Hardened", "Increased HP for enemies" };
        EnemyModifier em;
        em.max_hp = enemy_base_hp;
        em.hp = enemy_base_hp_max;
        m.enemy_m.push_back(em);
        modifiers.push_back(m);
    }
    { 
        MapModifier m = { "Alert", "Larger detection radius" };
        EnemyModifier em;
        em.activation_radius = 100.0f;
        m.enemy_m.push_back(em);
        modifiers.push_back(m);
    }
    {
        MapModifier m = { "Power", "More enemy damage" };
        ProjectileStatModifier pm;
        pm.projectile_damage = 2;
        m.target_proj_m.push_back(pm);
        modifiers.push_back(m);
    }
    {
        MapModifier m = { "Weakness", "Less player damage" };
        ProjectileStatModifier pm;
        pm.projectile_damage = -1;
        m.player_proj_m.push_back(pm);
        modifiers.push_back(m);
    }
}