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

    const std::vector<float> &get_attack_angles(const Attack &attack, const int &projectile_extra_count) {
        switch(projectile_extra_count) {
            case 0:
                return Projectile_angles[attack];
            case 1:
                return Extra_projectile_angles[attack];
            default: 
                ASSERT_WITH_MSG(false, "EXTRA PROJECTILES FOR THIS COUNT NOT IMPLEMENTED");
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