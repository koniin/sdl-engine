
#include "engine.h"
#include "renderer.h"



//// Tilemap collision expansions
/////////////////////////////////
/////////////////////////////////

struct Tile {
    int x, y;
    bool face_left = false;
    bool face_right = false;
    bool face_top = false;
    bool face_bottom = false;
};

struct CollisionData {
    Vector2 position;
    Vector2 velocity;
    Rectangle box;
};

std::vector<Tile> debug_overlapped;
unsigned overlapped_tiles_n = 0;
Tile overlapped_tiles[24];
void collision_set_overlapped_tiles(const TileMap &map, const float x, const float y, const unsigned w, const unsigned h) {
    overlapped_tiles_n = 0;
    debug_overlapped.clear();
    unsigned layer = 0;

    float left = x;
    float top = y;
    float right = left + w;
    float bottom = top + w;

    ASSERT_WITH_MSG(left >= 0 && top >= 0, "Left or right is less than zero when finding overlapped tiles!");

    unsigned leftTile = (unsigned)(left / map.tile_size);
    unsigned topTile = (unsigned)(top / map.tile_size);
    unsigned rightTile = (unsigned) Math::ceiling(right / map.tile_size) - 1;
    unsigned bottomTile = (unsigned) Math::ceiling((bottom / map.tile_size)) - 1;

    for (unsigned tile_y = topTile; tile_y <= bottomTile; ++tile_y){
        for (unsigned tile_x = leftTile; tile_x <= rightTile; ++tile_x){
            unsigned tile = map.tiles[Tiling::tilemap_index(map, layer, tile_x, tile_y)];

            Tile t;
            t.x = tile_x;
            t.y = tile_y;
            debug_overlapped.push_back(t);

            // ZERO IS SOLID TILE
            if(tile == 0) {
                overlapped_tiles[overlapped_tiles_n] = t;
                
                // SET FACES HERE ( TOP , LEFT, ... )
                if (tile_x - 1 > 0 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x - 1, tile_y)] != 0) {
                    overlapped_tiles[overlapped_tiles_n].face_left = true;
                }
                if (tile_x + 1 < map.columns 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x + 1, tile_y)] != 0) {
                    overlapped_tiles[overlapped_tiles_n].face_right = true;
                }
                if (tile_y - 1 > 0 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x, tile_y - 1)] != 0) {
                    overlapped_tiles[overlapped_tiles_n].face_top = true;
                }
                if (tile_y + 1 < map.rows 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x, tile_y + 1)] != 0) {
                    overlapped_tiles[overlapped_tiles_n].face_bottom = true;
                }
                ++overlapped_tiles_n;
            }
        }
    }
}

std::vector<Tile> debug_collided_tiles;
const int TILE_WIDTH = 16;

float tile_check_x(CollisionData &data, Tile &tile, Rectangle &tile_r);
float tile_check_y(CollisionData &data, Tile &tile, Rectangle &tile_r);

void resolve_collided_tiles(CollisionData &data) {
    debug_collided_tiles.clear();

    for(unsigned i = 0; i < overlapped_tiles_n; i++) {
        Rectangle tile_r = { overlapped_tiles[i].x * TILE_WIDTH, 
            overlapped_tiles[i].y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };

        if(tile_r.intersects(data.box)) {
            debug_collided_tiles.push_back(overlapped_tiles[i]);

            float minX = 0;
            float minY = 1;
            Tile tile = overlapped_tiles[i];

            if (Math::abs_f(data.velocity.x) > Math::abs_f(data.velocity.y)){
                //  Moving faster horizontally, check X axis first
                minX = -1;
            }
            else if (Math::abs_f(data.velocity.x) < Math::abs_f(data.velocity.y)) {
                //  Moving faster vertically, check Y axis first
                minY = -1;
            }

            if (data.velocity.x != 0 && data.velocity.y != 0 
                && (tile.face_left || tile.face_right) && (tile.face_bottom || tile.face_top)) {
                minX = (float)Math::min(Math::abs(data.box.x - tile_r.right()), Math::abs(data.box.right() - tile_r.left()));
                minY = (float)Math::min(Math::abs(data.box.y - tile_r.bottom()), Math::abs(data.box.bottom() - tile_r.top()));
            }
            
            float ox = 0;
            float oy = 0;
            if (minX < minY) {
                if (tile.face_left || tile.face_right) {
                    ox = tile_check_x(data, tile, tile_r);
                    //  That's horizontal done, check if we still intersects? If not then we can return now
                    if (ox != 0 && !tile_r.intersects(data.box)) {
                        continue;
                    }
                }

                if (tile.face_top || tile.face_bottom) {
                    oy = tile_check_y(data, tile, tile_r);
                }
            } else {
                if (tile.face_top || tile.face_bottom) {
                    oy = tile_check_y(data, tile, tile_r);
                    //  That's vertical done, check if we still intersects? If not then we can return now
                    if (oy != 0 && !tile_r.intersects(data.box)){
                        continue;
                    }
                }

                if (tile.face_left || tile.face_right) {
                    ox = tile_check_x(data, tile, tile_r);
                }
            }
        }
    }
}

float tile_check_y(CollisionData &data, Tile &tile, Rectangle &tile_r) {
    float oy = 0.0f;
    if (data.velocity.y < 0 && tile.face_bottom && data.box.y < tile_r.bottom()){
        oy = (float)data.box.y - tile_r.bottom();
    } else if (data.velocity.y > 0 && tile.face_top && data.box.bottom() > tile_r.top()){
        oy = (float)data.box.bottom() - tile_r.top();
    }
    if (oy != 0){
        data.position = Vector2(data.position.x, data.position.y - oy);
        data.velocity = Vector2(data.velocity.x, 0);
    }
    return oy;
}

float tile_check_x(CollisionData &data, Tile &tile, Rectangle &tile_r) {
    float ox = 0.0f;
    if (data.velocity.x < 0 && tile.face_right && data.box.x < tile_r.right()){
        ox = (float)data.position.x - tile_r.right();
    } else if (data.velocity.x > 0 &&  data.box.right() > tile_r.left()) {
        ox = (float)data.box.right() - tile_r.left();
    }
    if (ox != 0){
        data.position = Vector2(data.position.x - ox, data.position.y);
        data.velocity = Vector2(0, data.velocity.y);
    }
    return ox;
}
/////////////////////////////////
/////////////////////////////////

struct InputMapping {
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
	SDL_Scancode fire;
};

InputMapping input_maps[2] = {
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_SPACE },
	{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_ENTER }
};

struct PlayerInput {
    Vector2 move;
};

struct Velocity {
    Vector2 v;
};

struct Position {
    Vector2 p;
};

struct CollisionShape {
    Rectangle r;
};

struct BoxMan {
    PlayerInput input;
    Velocity velocity;
    Position position;
    CollisionShape collision_shape;
};

struct Settings {
    float player_move_speed = 0.2f;
    float player_max_speed = 1.0f;
    float movement_drag = 0.95f;
} settings;

unsigned box_n = 0;
std::vector<BoxMan> boxes(10);

void spawn_box() {
    BoxMan b;
    b.input.move = Vector2(0, 0);
    b.position.p = Vector2((float)gw / 2, (float)gh / 2);
    b.velocity.v = Vector2(0, 0);
    b.collision_shape.r = { 0, 0, 16, 16 };
    // b.collision_shape.r = { -8, -8, 16, 16 };
    boxes[box_n++] = b;
}

void tilemap_set_collisions(TileMap &t) {
    for(unsigned layer = 0; layer < t.layers; layer++) {
		for(unsigned y = 0; y < t.rows; y++) {
			for(unsigned x = 0; x < t.columns; x++) {
                if(x == 0 || y == 0 || x == t.columns - 1 || y == t.rows - 1) {
                    t.tiles[Tiling::tilemap_index(t, layer, x, y)] = 0;
                }
            }
        }
    }

    /*
     Log tilemap
    for(unsigned layer = 0; layer < t.layers; layer++) {
		for(unsigned y = 0; y < t.rows; y++) {
            Engine::log("\n");
			for(unsigned x = 0; x < t.columns; x++) {
                unsigned tile = t.tiles[Tiling::tilemap_index(t, layer, x, y)];
                Engine::log("  %d", tile);
            }
        }
    }
    */
}

void system_input() {
	InputMapping key_map = input_maps[0];

    for(unsigned i = 0; i < box_n; i++) {
        PlayerInput &pi = boxes[i].input;
        pi.move = Vector2::Zero;

        if(Input::key_down(key_map.up)) {
            pi.move.y = -1;
        } else if(Input::key_down(key_map.down)) {
            pi.move.y = 1;
        } 
        
        if(Input::key_down(key_map.left)) {
            pi.move.x = -1;
        } else if(Input::key_down(key_map.right)) {
            pi.move.x = 1;
        }
    }
}

void system_velocity() {
    for(unsigned i = 0; i < box_n; i++) {
        PlayerInput &pi = boxes[i].input;
        Velocity &v = boxes[i].velocity;
        v.v += pi.move * settings.player_move_speed;
    }
}

void system_physics(TileMap &t) {
    for(unsigned i = 0; i < box_n; i++) {
        Position &p = boxes[i].position;
        Velocity &v = boxes[i].velocity;

        p.p += v.v;
        // get the tiles
        
        collision_set_overlapped_tiles(t, 
            boxes[0].position.p.x + boxes[0].collision_shape.r.x, 
            boxes[0].position.p.y + boxes[0].collision_shape.r.y,
            boxes[0].collision_shape.r.w, 
            boxes[0].collision_shape.r.h);

        if(overlapped_tiles_n > 0) {
            Rectangle box_rect = {
                (int)(boxes[0].position.p.x + boxes[0].collision_shape.r.x), 
                (int)(boxes[0].position.p.y + boxes[0].collision_shape.r.y),
                (int)(boxes[0].collision_shape.r.w), 
                (int)(boxes[0].collision_shape.r.h)
            };

            CollisionData d;
            d.box = box_rect;
            d.velocity = v.v;
            d.position = p.p;

            resolve_collided_tiles(d);

            Engine::logn("\nold pos x: %f, y: %f", p.p.x, p.p.y);
            Engine::logn("old velocity x: %f, y: %f", v.v.x, v.v.y);
            Engine::logn("new pos x: %f, y: %f", d.position.x, d.position.y);
            Engine::logn("new velocity x: %f, y: %f \n", d.velocity.x, d.velocity.y);
            p.p = d.position;
            v.v = d.velocity;
        }
        
        v.v *= settings.movement_drag;
    }
}

TileMap tile_map;
void tile_collisions_load() {
    Tiling::tilemap_make(tile_map, 1, 40, 22, 16, 1);
    tilemap_set_collisions(tile_map);
    spawn_box();
}

void tile_collisions_update() {
    system_input();
    system_velocity();
    system_physics(tile_map);
}

void tile_collisions_render() {
    auto &t = tile_map;
    
    for(unsigned layer = 0; layer < t.layers; layer++) {        
		for(unsigned y = 0; y < t.rows; y++) {
			for(unsigned x = 0; x < t.columns; x++) {
				unsigned tile = t.tiles[Tiling::tilemap_index(t, layer, x, y)];
                if(tile == 1) {
                    draw_g_rectangle_filled_RGBA(x * t.tile_size, y * t.tile_size, t.tile_size, t.tile_size, 0, 200, 0, 255);
                } else if(tile == 0) {
                    draw_g_rectangle_filled_RGBA(x * t.tile_size, y * t.tile_size, t.tile_size, t.tile_size, 200, 20, 0, 255);
                } else {
                    
                }
			}
		}
	}

    for(unsigned i = 0; i < debug_overlapped.size(); i++) {
        draw_g_rectangle_filled_RGBA(debug_overlapped[i].x * tile_map.tile_size, 
                debug_overlapped[i].y * tile_map.tile_size, 
                tile_map.tile_size, tile_map.tile_size, 
                0, 0, 255, 255);
    }

    for(unsigned i = 0; i < debug_collided_tiles.size(); i++) {
        draw_g_rectangle_filled_RGBA(debug_collided_tiles[i].x * tile_map.tile_size, 
                debug_collided_tiles[i].y * tile_map.tile_size, 
                tile_map.tile_size, tile_map.tile_size, 
                120, 120, 120, 255);
        
        if(debug_overlapped[i].face_bottom) {
            draw_g_horizontal_line_RGBA(debug_overlapped[i].x * tile_map.tile_size,
            debug_overlapped[i].x * tile_map.tile_size + tile_map.tile_size,
            debug_overlapped[i].y * tile_map.tile_size,
            255, 0, 0, 255);
        }
    }

    for(unsigned i = 0; i < box_n; i++) {
        draw_g_rectangle_filled_RGBA(boxes[i].position.p.x + boxes[i].collision_shape.r.x, 
            boxes[i].position.p.y + boxes[i].collision_shape.r.y,
            boxes[i].collision_shape.r.w, boxes[i].collision_shape.r.h, 
            255, 255, 255, 255);
    }
}