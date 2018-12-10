#ifndef GAME_AREA_CONTROLLER_H
#define GAME_AREA_CONTROLLER_H

#include "game_area.h"
#include "game_data.h"

struct GameAreaController {
    GameArea *game_area;
    std::unordered_map<std::string, Sound::SoundId> sound_map;
    MapSettings map_settings;

    std::vector<ProjectileSpawn> player_projectile_queue;
    std::vector<ProjectileSpawn> target_projectile_queue;

    GameAreaController(GameArea *g) : game_area(g) {
        player_projectile_queue.reserve(256);
        target_projectile_queue.reserve(512);
    }

    void clear() {
        game_area->clear();
        player_projectile_queue.clear();
        target_projectile_queue.clear();
    }

    void set_world_bounds(const Rectangle &bounds) {
        game_area->world_bounds = bounds;
    }

    void set_settings(const MapSettings &settings) {
        map_settings = settings;
    }

    bool boss_spawned = false;
    Enemy level_boss;
    void set_boss(const Enemy &boss) {
        // place holder
        level_boss = boss;
        boss_spawned = false;
    }

    bool spawn_boss() {
        if(boss_spawned || game_area->targets.length > 0 || game_area->players.length == 0) {
            return false;
        }

        boss_spawned = true;
        
        // Get player position and spawn outside that
        auto player_pos = game_area->players.position[0].value;
        float minimum_spawn_distance = (float)gh;
        Vector2 boss_pos;;
        int count = 0;
        float buffer_area = 50;
        do {
            boss_pos = RNG::vector2(
                (float)game_area->world_bounds.x + buffer_area, 
                (float)game_area->world_bounds.right() - buffer_area, 
                (float)game_area->world_bounds.y + buffer_area, 
                (float)game_area->world_bounds.bottom()) - buffer_area;

            count++;
            if(count > 10) {
                boss_pos = Vector2((float)game_area->world_bounds.x / 2, (float)game_area->world_bounds.y / 2);
                break;
            }
        } while(Math::distance_f(boss_pos.x, boss_pos.y, player_pos.x, player_pos.y) < minimum_spawn_distance);

        Engine::logn("spawn tries: %d", count);
        Engine::logn("boss spawned at: %.1f, %.1f", boss_pos.x, boss_pos.y);

        spawn_target(boss_pos, level_boss);

        return true;
    }

    void set_background_color(const SDL_Color &color) {
        game_area->background_color = color;
    }

    void add_static_tile(const Vector2 &pos, const SpriteComponent &sprite) {
        game_area->tiles.push_back({ pos, sprite });
    }

    void spawn_projectiles();
    void spawn_effects();

    void spawn_player(const Vector2 &position);
    void spawn_target(const Vector2 &position, const Enemy &e);
    
    void spawn_muzzle_flash_effect(Vector2 p, Vector2 local_position, ECS::Entity parent);
    void spawn_explosion_effect(Vector2 position, float offset_x, float offset_y);
    void spawn_smoke_effect(Vector2 position) {
        game_area->smoke_emitter.position = position;
        Particles::emit(game_area->particles, game_area->smoke_emitter);
    }
    void spawn_hit_effect(Vector2 position, float angle);

    const bool game_over() {
        return game_area->players.length == 0;
    }

    const bool game_win() {
        return game_area->targets.length == 0;
    }

    void target_projectile_fire(const ProjectileSpawn &p) {
        target_projectile_queue.push_back(p);
    }

    // Order
    // 1. first we take the data from the attack and use as base
    // 2. apply player upgrade modifiers to the base
    // 3. apply map settings things to that
    // 4. then we fire the projectiles based on those numbers
    ProjectileFireResult player_projectile_fire(const int &ammo, const float &angle, const Vector2 &position) {
        GameState *game_state = GameData::game_state_get();
        const Attack &attack = game_state->player.attack;
        Attack_t t_attack = Attacks[attack];

        int p_extra_count = 0;
        for(auto &upgrade : game_state->player_upgrades) {
            upgrade.apply_projectile_modifiers(t_attack);
            p_extra_count += upgrade.count_extra_projectiles();
        }

        map_settings.apply_player_projectile_modifiers(t_attack);
        
        char *sound_name = t_attack.sound_name;
        auto fire_result = ProjectileFireResult(t_attack.cooldown, t_attack.knockback, sound_name);

        int ammo_usage = t_attack.ammo;
        if(ammo_usage <= ammo) {
            fire_result.ammo_used = ammo_usage;
            fire_result.did_fire = true;
            
            switch(t_attack.type) {
                case ProjectileAttack: {
                    const std::vector<float> &angles = GameData::get_attack_angles(attack, p_extra_count);
                    ASSERT_WITH_MSG(angles.size() > 0, "NO ANGLES DEFINED FOR THIS ATTACK and extra count!");
                    attack_spawn_projectiles(t_attack, position, angle, angles);
                    break;
                }
                case LineAttack: {
                    attack_spawn_line_attack(t_attack, position, angle);
                    break;
                }
                default: {
                    ASSERT_WITH_MSG(false, "No spawn defined for this attack type");
                    break;
                }
            }
        } 
        
        return fire_result;
    }

    void attack_spawn_projectiles(Attack_t &t_attack, const Vector2 &position, const float &initial_angle, const std::vector<float> &angles) {
        const float &accuracy = t_attack.accuracy;
        const float &time_to_live = t_attack.range;
        const float &projectile_speed_mod = t_attack.projectile_speed_mod;

        for(auto &angle_offset : angles) {
            float final_angle = initial_angle + angle_offset + RNG::range_f(-accuracy, accuracy);
            float final_speed = t_attack.projectile_speed + RNG::range_f(-projectile_speed_mod, projectile_speed_mod);
            ProjectileSpawn p(position, final_angle, final_speed, t_attack.projectile_damage, t_attack.projectile_radius, 
                time_to_live, t_attack.pierce_count, t_attack.split_count);
            p.homing_radius = t_attack.homing_radius;
            p.explosion_on_death_radius = t_attack.projectile_death_explosion_radius;
            p.explosion_on_hit_radius = t_attack.projectile_hit_explosion_radius;
            
            player_projectile_queue.push_back(p);
        }
    }

    void calc_lazer(SDL_Rect &lazer_rect, const Vector2 &start, const Vector2 &end, const int &height) {
        float distance = Math::distance_v(start, end);
        Vector2 difference = end - start;
        lazer_rect.x = (int)(start.x + (difference.x / 2) - (distance / 2));
        lazer_rect.y = (int)(start.y + (difference.y / 2) - (height / 2));
        lazer_rect.w = (int)distance;
        lazer_rect.h = height;
    }

    void attack_spawn_line_attack(Attack_t &t_attack, const Vector2 &position, const float &initial_angle) {
        Vector2 end_point;
        ray_cast_targets_for_end_point(position, initial_angle, 200.0f, end_point);
        
        SDL_Rect lazer_rect;
        int height = 8;
        calc_lazer(lazer_rect, end_point, position, height);
        
        // Adjust for rendering at center of sprite
        lazer_rect.x = lazer_rect.x + (lazer_rect.w / 2);
        lazer_rect.y = lazer_rect.y + (lazer_rect.h / 2);

        auto final_speed = 0.0f;
        auto ttl = Time::delta_time_fixed * 8;
        ProjectileSpawn p(Vector2((float)lazer_rect.x, (float)lazer_rect.y), initial_angle, final_speed, t_attack.projectile_damage, t_attack.projectile_radius, 
                ttl, t_attack.pierce_count, t_attack.split_count);
            // p.homing_radius = t_attack.homing_radius;
        p.explosion_on_death_radius = t_attack.projectile_death_explosion_radius;
        p.explosion_on_hit_radius = t_attack.projectile_hit_explosion_radius;
        p.test_rect = lazer_rect;
        player_projectile_queue.push_back(p);
    }

    void player_projectile_hit(const ECS::Entity &player_projectile, const ECS::Entity &target_entity, const Vector2 &collision_point) {
        auto handle = game_area->projectiles_player.get_handle(player_projectile);
        if(game_area->projectiles_player.is_valid(handle)) {
            int split_count = game_area->projectiles_player.split[handle.i].count;
            if(split_count > 0) {
                split_player_projectile(map_settings, split_count, collision_point);
            }

            float on_hit_explosion_radius = game_area->projectiles_player.on_hit[handle.i].explosion_radius;
            if(on_hit_explosion_radius > 0) {
                spawn_player_explosion_projectile(game_area->projectiles_player.position[handle.i].value);
            }
        }
    }

    void split_player_projectile(const MapSettings &settings, const int &count, const Vector2 &position) {
        GameState *game_state = GameData::game_state_get();
        const Attack &attack = game_state->player.attack;
        Attack_t t_attack = Attacks[attack];

        int p_extra_count = 0;
        for(auto &upgrade : game_state->player_upgrades) {
            upgrade.apply_projectile_modifiers(t_attack);
            p_extra_count += upgrade.count_extra_projectiles();
        }

        map_settings.apply_player_projectile_modifiers(t_attack);
    
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
            player_projectile_queue.push_back(p);
        }
    }

    void spawn_player_explosion_projectile(const Vector2 pos) {
        auto p = ProjectileSpawn(pos, 0, 0, 10, 20, Time::delta_time_fixed * 8, 200, 0);
        player_projectile_queue.push_back(p);

        spawn_explosion_effect(pos, 0, 0);
    }

    void ray_cast_targets_for_end_point(const Vector2 &position, const float &angle, const float &search_length, Vector2 &target_point) {
        Vector2 line_search_end = Math::direction_from_angle(angle) * search_length;
        Vector2 end_point = position + line_search_end;
        target_point = end_point;
        float dist = 9000.0f;
        for(int j = 0; j < game_area->targets.length; ++j) {
            const Vector2 &t_pos = game_area->targets.position[j].value;
            const float &t_radius = (float)game_area->targets.collision[j].radius;
            Vector2 entry_point;
            int result = Intersects::line_circle_entry(position, end_point, t_pos, t_radius, entry_point);
            if(result == 1 || result == 2) {
                auto dir = Math::direction_from_angle(angle);
                Vector2 collision_point = t_pos + (t_radius * -dir);
                float cur_dist = Math::distance_v(position, collision_point);
                if(cur_dist < dist) {
                    dist = cur_dist;
                    target_point = collision_point;
                }
            }
        }
    }
};

#endif