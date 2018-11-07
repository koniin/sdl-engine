#ifndef RENDERING_H
#define RENDERING_H

#include "engine.h"
#include "renderer.h"

struct SpriteData {
    int16_t x, y;
    int w, h;
    SDL_Color color;
    int sprite_index;
    std::string sprite_name;
    float rotation;
    int layer;

    bool operator<(const SpriteData &rhs) const { 
        return layer < rhs.layer; 
    }
};
struct RenderBuffer {    
    int sprite_count = 0;
    SpriteData *sprite_data_buffer;
};
template<typename T>
void export_sprite_data(const T &entity_data, const int i, SpriteData &spr) {
    // handle camera, zoom and stuff here

    // also we can do culling here
    // intersects world_bounds etc

    // float globalScale = 0.05f;
    // spr.x = go.pos.x * globalScale;
    // spr.y = go.pos.y * globalScale;
    // spr.scale = go.sprite.scale * globalScale;
    // spr.x = entity_data.position[i].x - camera.x;
    // spr.x = entity_data.position[i].y - camera.y;

    const auto &camera = get_camera();

    spr.x = (int16_t)(entity_data.position[i].value.x - camera.x);
    spr.y = (int16_t)(entity_data.position[i].value.y - camera.y);
    spr.w = entity_data.sprite[i].w;
    spr.h = entity_data.sprite[i].h;
    spr.sprite_index = entity_data.sprite[i].sprite_sheet_index;
    spr.sprite_name = entity_data.sprite[i].sprite_name;
    spr.rotation = entity_data.sprite[i].rotation;
    spr.layer = entity_data.sprite[i].layer;
}

template<typename T>
void export_sprite_data_values(const Vector2 &position, const T &sprite, const int i, SpriteData &spr) {
    // handle camera, zoom and stuff here

    // also we can do culling here
    // intersects world_bounds etc

    // float globalScale = 0.05f;
    // spr.x = go.pos.x * globalScale;
    // spr.y = go.pos.y * globalScale;
    // spr.scale = go.sprite.scale * globalScale;
    // spr.x = entity_data.position[i].x - camera.x;
    // spr.x = entity_data.position[i].y - camera.y;

    const auto &camera = get_camera();

    spr.x = (int16_t)(position.x - camera.x);
    spr.y = (int16_t)(position.y - camera.y);
    spr.w = sprite.w;
    spr.h = sprite.h;
    spr.sprite_index = sprite.sprite_sheet_index;
    spr.sprite_name = sprite.sprite_name;
    spr.rotation = sprite.rotation;
    spr.layer = sprite.layer;
}

void draw_buffer(const SpriteData *spr, const int length) {
    const std::vector<SpriteSheet> &sprite_sheets = Resources::get_sprite_sheets();
    for(int i = 0; i < length; i++) {
        draw_spritesheet_name_centered_ex(sprite_sheets[spr[i].sprite_index], spr[i].sprite_name, spr[i].x, spr[i].y, spr[i].w, spr[i].h, spr[i].rotation);
         //draw_spritesheet_name_centered_rotated(sprite_sheets[spr[i].sprite_index], spr[i].sprite_name, spr[i].x, spr[i].y, spr[i].rotation);
    }
}

#endif