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

    const std::vector<float> &get_angles(const Attack &attack, const int &projectile_extra_count) {
        switch(projectile_extra_count) {
            case 0:
                return Projectile_angles[attack];
            case 1:
                return Extra_projectile_angles[attack];
            default: 
                ASSERT_WITH_MSG(false, "EXTRA PROJECTILES FOR THIS COUNT NOT IMPLEMENTED");
        }
    }

    // Order
    // 1. first we take the data from the attack and use as base
    // 2. apply player upgrade modifiers to the base
    // 3. apply map settings things to that
    // 4. then we fire the projectiles based on those numbers
    ProjectileFireResult trigger_projectile_fire(const int &ammo, const MapSettings &settings, 
        float angle, Vector2 pos, std::vector<ProjectileSpawn> &projectiles_queue) 
    {
        const Attack &attack = game_state->player.attack;
        Attack_t t_attack = Attacks[attack];

        int p_extra_count = 0;
        for(auto &upgrade : game_state->player_upgrades) {
            upgrade.apply_projectile_modifiers(t_attack);
            p_extra_count += upgrade.count_extra_projectiles();
        }

        settings.apply_player_projectile_modifiers(t_attack);
        
        char *sound_name = t_attack.sound_name;
        auto fire_result = ProjectileFireResult(t_attack.cooldown, t_attack.knockback, sound_name);

        int ammo_usage = t_attack.ammo;
        if(ammo_usage <= ammo) {
            fire_result.ammo_used = ammo_usage;
            fire_result.did_fire = true;
            // find angles
            const std::vector<float> &angles = get_angles(attack, p_extra_count);
            ASSERT_WITH_MSG(angles.size() > 0, "NO ANGLES DEFINED FOR THIS ATTACK and extra count!");
            
            const float &accuracy = t_attack.accuracy;
            const float &time_to_live = t_attack.range;
            const float &projectile_speed_mod = t_attack.projectile_speed_mod;

            for(auto &angle_offset : angles) {
                float final_angle = angle + angle_offset + RNG::range_f(-accuracy, accuracy);
                float final_speed = t_attack.projectile_speed + RNG::range_f(-projectile_speed_mod, projectile_speed_mod);
                ProjectileSpawn p(pos, final_angle, final_speed, t_attack.projectile_damage, t_attack.projectile_radius, 
                    time_to_live, t_attack.pierce_count, t_attack.split_count);
                p.homing_radius = t_attack.homing_radius;
                p.explosion_on_death_radius = t_attack.projectile_death_explosion_radius;
                p.explosion_on_hit_radius = t_attack.projectile_hit_explosion_radius;
                projectiles_queue.push_back(p);
            }
        } 
        
        return fire_result;
    }

    void split_player_projectile(const MapSettings &settings, const int &count, const Vector2 &position, std::vector<ProjectileSpawn> &projectiles_queue) {
        const Attack &attack = game_state->player.attack;
        Attack_t t_attack = Attacks[attack];

        int p_extra_count = 0;
        for(auto &upgrade : game_state->player_upgrades) {
            upgrade.apply_projectile_modifiers(t_attack);
            p_extra_count += upgrade.count_extra_projectiles();
        }

        settings.apply_player_projectile_modifiers(t_attack);
    
        float angle = RNG::range_f(0, 360);
        float final_speed = bp_spd();
        int damage = 2;
        int radius = 8;
        float time_to_live = 0.4f;
        int pierce_count = 0;
        int split_count = 0; // don't want projectiles to split recursively?

        // explosions for split projectiles ? ;)

        size_t max_split = Math::min_i(count + p_extra_count, split_angles.size());
        for(size_t i = 0; i < max_split; i++) {
            float final_angle = angle + split_angles[i];
            ProjectileSpawn p(position, final_angle, final_speed, damage, radius, time_to_live, pierce_count, split_count);
            projectiles_queue.push_back(p);
        }
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