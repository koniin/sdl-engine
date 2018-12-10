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

    bool spawn_boss();

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
    
    ProjectileFireResult player_projectile_fire(const int &ammo, const float &angle, const Vector2 &position);
    void attack_spawn_projectiles(Attack_t &t_attack, const Vector2 &position, const float &initial_angle, const std::vector<float> &angles);
    void attack_spawn_line_attack(Attack_t &t_attack, const Vector2 &position, const float &initial_angle);
    void player_projectile_hit(const ECS::Entity &player_projectile, const ECS::Entity &target_entity, const Vector2 &collision_point);
    void split_player_projectile(const MapSettings &settings, const int &count, const Vector2 &position);
    void spawn_player_explosion_projectile(const Vector2 pos);
    void ray_cast_targets_for_end_point(const Vector2 &position, const float &angle, const float &search_length, Vector2 &target_point);
};

#endif