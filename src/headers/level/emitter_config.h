#ifndef EMITTER_CONFIG_H
#define EMITTER_CONFIG_H

#include "engine.h"
#include "renderer.h"

template<typename T>
void emitters_configure(T *emitter_container) {
    emitter_container->explosion_emitter.position = Vector2(320, 180);
    emitter_container->explosion_emitter.color_start = Colors::make(229,130,0,255);
    emitter_container->explosion_emitter.color_end = Colors::make(255,255,255,255);
    emitter_container->explosion_emitter.force = Vector2(10, 22);
    emitter_container->explosion_emitter.min_particles = 8;
    emitter_container->explosion_emitter.max_particles = 12;
    emitter_container->explosion_emitter.life_min = 0.100f;
    emitter_container->explosion_emitter.life_max = 0.250f;
    emitter_container->explosion_emitter.angle_min = 0;
    emitter_container->explosion_emitter.angle_max = 360;
    emitter_container->explosion_emitter.speed_min = 32;
    emitter_container->explosion_emitter.speed_max = 56;
    emitter_container->explosion_emitter.size_min = 1.600f;
    emitter_container->explosion_emitter.size_max = 5;
    emitter_container->explosion_emitter.size_end_min = 6.200f;
    emitter_container->explosion_emitter.size_end_max = 9;

    emitter_container->hit_emitter.position = Vector2(320, 180);
    emitter_container->hit_emitter.color_start = Colors::make(229,130,0,255);
    emitter_container->hit_emitter.color_end = Colors::make(255,255,255,255);
    emitter_container->hit_emitter.force = Vector2(0, 0);
    emitter_container->hit_emitter.min_particles = 30;
    emitter_container->hit_emitter.max_particles = 42;
    emitter_container->hit_emitter.life_min = 0.100f;
    emitter_container->hit_emitter.life_max = 0.200f;
    emitter_container->hit_emitter.angle_min = 0;
    emitter_container->hit_emitter.angle_max = 32.400f;
    emitter_container->hit_emitter.speed_min = 122;
    emitter_container->hit_emitter.speed_max = 138;
    emitter_container->hit_emitter.size_min = 2.200f;
    emitter_container->hit_emitter.size_max = 2.600f;
    emitter_container->hit_emitter.size_end_min = 0;
    emitter_container->hit_emitter.size_end_max = 0.600f;

    emitter_container->exhaust_emitter.position = Vector2(320, 180);
    emitter_container->exhaust_emitter.color_start = Colors::make(229,130,0,255);
    emitter_container->exhaust_emitter.color_end = Colors::make(255,255,255,255);
    emitter_container->exhaust_emitter.force = Vector2(10, 10);
    emitter_container->exhaust_emitter.min_particles = 26;
    emitter_container->exhaust_emitter.max_particles = 34;
    emitter_container->exhaust_emitter.life_min = 0.100f;
    emitter_container->exhaust_emitter.life_max = 0.200f;
    emitter_container->exhaust_emitter.angle_min = 0;
    emitter_container->exhaust_emitter.angle_max = 10.800f;
    emitter_container->exhaust_emitter.speed_min = 106;
    emitter_container->exhaust_emitter.speed_max = 130;
    emitter_container->exhaust_emitter.size_min = 0.600f;
    emitter_container->exhaust_emitter.size_max = 1.400f;
    emitter_container->exhaust_emitter.size_end_min = 1.800f;
    emitter_container->exhaust_emitter.size_end_max = 3;

    emitter_container->smoke_emitter.position = Vector2(320, 180);
    emitter_container->smoke_emitter.color_start = Colors::make(165, 165, 165, 255);
    emitter_container->smoke_emitter.color_end = Colors::make(0, 0, 0, 255);
    emitter_container->smoke_emitter.force = Vector2(0, 0);
    emitter_container->smoke_emitter.min_particles = 26;
    emitter_container->smoke_emitter.max_particles = 34;
    emitter_container->smoke_emitter.life_min = 0.200f;
    emitter_container->smoke_emitter.life_max = 0.400f;
    emitter_container->smoke_emitter.angle_min = 0;
    emitter_container->smoke_emitter.angle_max = 320.400f;
    emitter_container->smoke_emitter.speed_min = 3;
    emitter_container->smoke_emitter.speed_max = 6;
    emitter_container->smoke_emitter.size_min = 2;
    emitter_container->smoke_emitter.size_max = 4;
    emitter_container->smoke_emitter.size_end_min = 1;
    emitter_container->smoke_emitter.size_end_max = 2;
}

#endif