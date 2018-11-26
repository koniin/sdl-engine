#ifndef GAME_AREA_CONTROLLER_H
#define GAME_AREA_CONTROLLER_H

#include "game_area.h"

struct GameAreaController {
    GameArea *game_area;
    std::unordered_map<std::string, Sound::SoundId> sound_map;

    GameAreaController(GameArea *g) : game_area(g) {}

    void set_world_bounds(const Rectangle &bounds) {
        game_area->world_bounds = bounds;
    }

    bool boss_spawned = false;
    void set_boss(int id) {
        // place holder
    }

    void spawn_boss() {
        if(boss_spawned) {
            return;
        }

        if(game_area->targets.length == 0) {
            boss_spawned = true;    
            // Get player position and spawn outside that
            if(game_area->players.length > 0) {
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
                spawn_target(boss_pos);
            }
        }
    }

    void set_background_color(const SDL_Color &color) {
        game_area->background_color = color;
    }

    void add_static_tile(const Vector2 &pos, const SpriteComponent &sprite) {
        game_area->tiles.push_back({ pos, sprite });
    }

    void spawn_projectiles();
    void spawn_effects();

    void spawn_player(Vector2 position);
    void spawn_target(Vector2 position);
    
    void spawn_player_projectile(Vector2 pos, Vector2 velocity);
    void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent);
    void spawn_explosion(Vector2 position, float offset_x, float offset_y);
    void spawn_smoke(Vector2 position) {
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
};

#endif