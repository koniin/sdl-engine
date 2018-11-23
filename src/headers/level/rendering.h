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

    void init(size_t sz) {
        sprite_data_buffer = new SpriteData[sz];
    }

    void clear() {
        sprite_count = 0;
    }
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
bool export_sprite_data_values_cull(const Vector2 &position, const T &sprite, const int i, SpriteData &spr) {
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
    int16_t x = (int16_t)(position.x - camera.x);
    int16_t y = (int16_t)(position.y - camera.y);
    int w = sprite.w;
    int h = sprite.h;

    Rectangle view;
    view.x = (int)camera.x - (gw / 2);
    view.y = (int)camera.y - (gh / 2);
    view.w = gw;
    view.h = gh;

    Rectangle item;
    item.x = x;
    item.y = y;
    item.w = w;
    item.h = h;

    if(!view.intersects(item)) {
        return false;
    }
    
    spr.x = x;
    spr.y = y;
    spr.w = w;
    spr.h = h;
    spr.sprite_index = sprite.sprite_sheet_index;
    spr.sprite_name = sprite.sprite_name;
    spr.rotation = sprite.rotation;
    spr.layer = sprite.layer;

    return true;
}

void draw_buffer(const SpriteData *spr, const int length) {
    const std::vector<SpriteSheet> &sprite_sheets = Resources::get_sprite_sheets();
    for(int i = 0; i < length; i++) {
        draw_spritesheet_name_centered_ex(sprite_sheets[spr[i].sprite_index], spr[i].sprite_name, spr[i].x, spr[i].y, spr[i].w, spr[i].h, spr[i].rotation);
         //draw_spritesheet_name_centered_rotated(sprite_sheets[spr[i].sprite_index], spr[i].sprite_name, spr[i].x, spr[i].y, spr[i].rotation);
    }
}

void export_render_info(RenderBuffer &render_buffer, GameArea *_g) {
    render_buffer.sprite_count = 0;
    auto sprite_data_buffer = render_buffer.sprite_data_buffer;
    auto &sprite_count = render_buffer.sprite_count;

    auto &tiles = _g->tiles;
    for(size_t i = 0; i < tiles.size(); i++) {
        if(export_sprite_data_values_cull(tiles[i].position, tiles[i].sprite, i, sprite_data_buffer[sprite_count])) {
            sprite_count++;
        }
    }

    for(int i = 0; i < _g->players.length; i++) {
        Direction &d = _g->players.direction[i];
        _g->players.sprite[i].rotation = d.angle + 90; // sprite is facing upwards so we need to adjust
        export_sprite_data(_g->players, i, sprite_data_buffer[sprite_count++]);
    }

    for(size_t i = 0; i < _g->players.child_sprites.length; ++i) {
        export_sprite_data(_g->players.child_sprites, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data_values(players.child_sprites.position[i], players.child_sprites[i].sprite, i, sprite_data_buffer[sprite_count++]);
    }

    for(int i = 0; i < _g->projectiles_player.length; ++i) {
        export_sprite_data(_g->projectiles_player, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < _g->projectiles_target.length; ++i) {
        export_sprite_data(_g->projectiles_target, i, sprite_data_buffer[sprite_count++]);
	}

    for(int i = 0; i < _g->targets.length; ++i) {
        export_sprite_data(_g->targets, i, sprite_data_buffer[sprite_count++]);
	}
    
    for(size_t i = 0; i < _g->targets.child_sprites.length; ++i) {
        export_sprite_data(_g->targets.child_sprites, i, sprite_data_buffer[sprite_count++]);
        // export_sprite_data_values(players.child_sprites.position[i], players.child_sprites[i].sprite, i, sprite_data_buffer[sprite_count++]);
    }

    for(int i = 0; i < _g->effects.length; ++i) {
        export_sprite_data(_g->effects, i, sprite_data_buffer[sprite_count++]);
	}

    // Sort the render buffer by layer
    std::sort(sprite_data_buffer, sprite_data_buffer + sprite_count);
}
#endif