#ifndef GAME_AREA_CONTROLLER_H
#define GAME_AREA_CONTROLLER_H

#include "game_area.h"

struct GameAreaController {
    GameArea *_g;

    GameAreaController(GameArea *g) : _g(g) {}
    
    void spawn_projectiles();
    void spawn_effects();

    void spawn_player(Vector2 position);
    void spawn_target(Vector2 position);
    
    void spawn_player_projectile(Vector2 pos, Vector2 velocity);
    void spawn_muzzle_flash(Position p, Vector2 local_position, ECS::Entity parent);
    void spawn_explosion(Vector2 position, float offset_x, float offset_y);
    void spawn_smoke(Vector2 position) {
        _g->smoke_emitter.position = position;
        Particles::emit(_g->particles, _g->smoke_emitter);
    }
    void spawn_hit_effect(Vector2 position, float angle);
};

#endif