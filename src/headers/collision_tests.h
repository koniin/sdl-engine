#ifndef COLLISION_TESTS_H
#define COLLISION_TESTS_H

#include "engine.h"
#include "renderer.h"

struct Circle {
    Vector2 position;
    uint16_t radius;
    Vector2 velocity;
};

struct Gun {
    Vector2 position;
    float angle;
} gun;

constexpr float player_bullet_speed() {
    return 8.0f / 0.016667f;
}

const int max_circles = 10;
static int circle_n = 0;
Circle *circles;

const int max_bullets = 10;
static int bullet_n = 0;
Circle *bullets;
float bullet_velocity_delta = player_bullet_speed();
float bullet_velocity = 8.0f;
uint16_t bullet_radius = 4;
bool use_delta_time_speed = true;
Rectangle world_bounds;

float bullet_life = 0.0f;

void spawn_circles() {
    int circle_max_size = 12;
    circle_n = 0;
    for(int i = 0; i < max_circles; i++) {
        circles[i].position = RNG::vector2((float)circle_max_size, (float)gw, (float)circle_max_size, (float)gh);
        circles[i].radius = (uint16_t)RNG::range_i(3, circle_max_size);
        ++circle_n;
    }
}

void fire_bullet() {
    if(bullet_n < max_bullets - 1) {
        bullets[bullet_n].position = gun.position;
        float rotation = gun.angle / Math::RAD_TO_DEGREE;
        float x_direction, y_direction;
        x_direction = cos(rotation);
        y_direction = sin(rotation);
        if(use_delta_time_speed) {
            bullets[bullet_n].velocity = Vector2(x_direction * bullet_velocity_delta, y_direction * bullet_velocity_delta);
        } else {
            bullets[bullet_n].velocity = Vector2(x_direction * bullet_velocity, y_direction * bullet_velocity);
        }
        bullets[bullet_n].radius = bullet_radius;
        ++bullet_n;

        bullet_life = 0;
    }
}

void collision_test_load() {
    Engine::set_base_data_folder("data");
    Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);

    circles = new Circle[max_circles];
    circles[0].position = Vector2((float)gw / 2, (float)gh / 2);
    circles[0].radius = 6;
    circle_n = 1;
    bullets = new Circle[max_bullets];

    gun.position = Vector2(200, (float)gh / 2);
    gun.angle = 0;

    world_bounds = { 0, 0, (int)gw, (int)gh };
}

void collision_test_update() {
    FrameLog::reset();
    
    if(Input::key_down(SDL_SCANCODE_A)) {
        gun.angle -= 5;
    } else if(Input::key_down(SDL_SCANCODE_D)) {
        gun.angle += 5;
    }

    if(Input::key_pressed(SDLK_SPACE)) {
        fire_bullet();
    }
    if(Input::key_pressed(SDLK_k)) {
        use_delta_time_speed = !use_delta_time_speed;
    }

    
    for(int i = 0; i < bullet_n; i++) {
        if(i == 0) {
            bullet_life += Time::deltaTime;
        }
        if(use_delta_time_speed) {
            bullets[i].position += bullets[i].velocity * Time::deltaTime;
        } else {
            bullets[i].position += bullets[i].velocity;
        }
    }

    Rectangle bounds = world_bounds;
    for(int i = 0; i < bullet_n; i++) {
        const auto &pos = bullets[i].position;
        if(pos.x < bounds.x || pos.x > bounds.right() || pos.y < bounds.y || pos.y > bounds.bottom()) {
            bullets[i] = bullets[--bullet_n];
        }
    }

    std::string delta = use_delta_time_speed ? "yes" : "no";
    FrameLog::log("Delta movement: " + delta);
    FrameLog::log("Bullet count: " + std::to_string(bullet_n));
    FrameLog::log("Bullet velocity: " + std::to_string(bullet_velocity));
    FrameLog::log("Bullet velocity_delta: " + std::to_string(bullet_velocity_delta));
    FrameLog::log("Bullet radius: " + std::to_string(bullet_radius));
    FrameLog::log("Bullet life: " + std::to_string(bullet_life));
}

void collision_test_render() {
    float x_direction, y_direction;
    float rotation = gun.angle / Math::RAD_TO_DEGREE;
    x_direction = cos(rotation);
    y_direction = sin(rotation);
    int x2 = (int)(gun.position.x + x_direction * 10);
    int y2 = (int)(gun.position.y + y_direction * 10);

    SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 255, 255);
    SDL_RenderDrawLine(renderer.renderer, (int)gun.position.x, (int)gun.position.y, x2, y2);
    for(int i = 0; i < circle_n; i++) {
        const auto &pos = circles[i].position;
        draw_g_circe_RGBA((uint16_t)pos.x, (uint16_t)pos.y, circles[i].radius, 0, 255, 0, 255);
    }
    for(int i = 0; i < bullet_n; i++) {
        const auto &pos = bullets[i].position;
        draw_g_circe_RGBA((uint16_t)pos.x, (uint16_t)pos.y, bullets[i].radius, 0, 255, 0, 255);
    }

    FrameLog::render(5, 5);
}

#endif