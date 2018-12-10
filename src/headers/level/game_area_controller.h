#ifndef GAME_AREA_CONTROLLER_H
#define GAME_AREA_CONTROLLER_H

#include "game_area.h"
#include "game_data.h"

struct GameAreaController {
    GameArea *game_area;
    std::unordered_map<std::string, Sound::SoundId> sound_map;
    MapSettings map_settings;

    GameAreaController(GameArea *g) : game_area(g) {}

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
        do {
            boss_pos = RNG::vector2(
                (float)game_area->world_bounds.x, 
                (float)game_area->world_bounds.right(), 
                (float)game_area->world_bounds.y, 
                (float)game_area->world_bounds.bottom());

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

    ProjectileFireResult player_projectile_fire(const int &ammo, const float &angle, const Vector2 &position) {
        return GameData::trigger_projectile_fire(ammo, map_settings, angle, position, game_area->projectiles_player.projectile_queue);
    }

    
    void calc_lazer(SDL_Rect &lazer_rect, const Vector2 &start, const Vector2 &end, const int &height) {
        float distance = Math::distance_v(start, end);
        Vector2 difference = end - start;
        lazer_rect.x = start.x + (difference.x / 2) - (distance / 2);
        lazer_rect.y = start.y + (difference.y / 2) - (height / 2);
        lazer_rect.w = distance;
        lazer_rect.h = height;
    }

    ProjectileFireResult player_projectile_line_fire(const int &ammo, const float &angle, const Vector2 &position) {
        float search_length = 200.0f;
        Vector2 line_search_end = Math::direction_from_angle(angle) * search_length;
        Vector2 end_point = position + line_search_end;
        float dist = 9000.0f;
        
        for(int j = 0; j < game_area->targets.length; ++j) {
            const Vector2 &t_pos = game_area->targets.position[j].value;
            const float &t_radius = game_area->targets.collision[j].radius;
            Vector2 entry_point;
            int result = Intersects::line_circle_entry(position, end_point, t_pos, t_radius, entry_point);
            if(result == 1 || result == 2) {
                auto dir = Math::direction_from_angle(angle);
                Vector2 collision_point = t_pos + (t_radius * -dir);
                float cur_dist = Math::distance_v(position, collision_point);
                if(cur_dist < dist) {
                    dist = cur_dist;
                    end_point = collision_point;
                }
            }
        }

        Vector2 start = end_point;
        Vector2 end = position;
        SDL_Rect lazer_rect;
        float height = 8;
        calc_lazer(lazer_rect, start, end, height);
        
        // Adjust for rendering at center of sprite
        lazer_rect.x = lazer_rect.x + (lazer_rect.w / 2);
        lazer_rect.y = lazer_rect.y + (lazer_rect.h / 2);

        auto p = ProjectileSpawn(Vector2(lazer_rect.x, lazer_rect.y), angle, 0, 0, 4, Time::delta_time_fixed * 8, 0, 0);
        p.test_rect = lazer_rect;
        game_area->projectiles_player.queue_projectile(p);

        ProjectileFireResult f = ProjectileFireResult(0.25f, 0, "basic_fire");
        f.did_fire = true;
        f.ammo_used = 1;
        return f;
    }

    void player_projectile_hit(const ECS::Entity &player_projectile, const ECS::Entity &target_entity, const Vector2 &collision_point) {
        auto handle = game_area->projectiles_player.get_handle(player_projectile);
        if(game_area->projectiles_player.is_valid(handle)) {
            int split_count = game_area->projectiles_player.split[handle.i].count;
            if(split_count > 0) {
                GameData::split_player_projectile(map_settings, split_count, collision_point, game_area->projectiles_player.projectile_queue);
            }

            float on_hit_explosion_radius = game_area->projectiles_player.on_hit[handle.i].explosion_radius;
            if(on_hit_explosion_radius > 0) {
                spawn_player_explosion_projectile(game_area->projectiles_player.position[handle.i].value);
            }
        }
    }

    void spawn_player_explosion_projectile(const Vector2 pos) {
        auto p = ProjectileSpawn(pos, 0, 0, 10, 20, Time::delta_time_fixed * 8, 200, 0);
        game_area->projectiles_player.queue_projectile(p);

        spawn_explosion_effect(pos, 0, 0);
    }
};

#endif