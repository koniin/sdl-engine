
#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

#include "engine.h"
#include "renderer.h"

// Pixels per frame
constexpr float player_bullet_speed() {
    return 8.0f / 0.016667f;
}
constexpr float player_move_acceleration() {
    return 10.0f / 0.016667f;
}

struct PlayerConfiguration {
    int16_t radius = 8;
	float rotation_speed = 3.0f; // degrees
	float move_acceleration = player_move_acceleration();
	float drag = 0.04f;
	float fire_cooldown = 0.15f; // s
	float bullet_speed = player_bullet_speed();
    float gun_barrel_distance = 11.0f; // distance from center
    float fire_knockback = 2.0f; // pixels
    float fire_knockback_camera = -6.0f;
    int exhaust_id = 1;
};

struct TargetConfiguration {
    float knockback_on_hit = 2.0f;
};

static const size_t RENDER_BUFFER_MAX = 256;

void load_shooter();
void update_shooter();
void render_shooter();
void render_shooter_ui();


#endif