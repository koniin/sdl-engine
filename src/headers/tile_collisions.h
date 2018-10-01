
#include "engine.h"
#include "renderer.h"

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
    float player_move_speed = 0.5f;
    float player_max_speed = 1.0f;
    float movement_drag = 0.95f;
} settings;

unsigned box_n = 0;
std::vector<BoxMan> boxes(10);

void spawn_box() {
    BoxMan b;
    b.input.move = Vector2(0, 0);
    b.position.p = Vector2(gw / 2, gh / 2);
    b.velocity.v = Vector2(0, 0);
    b.collision_shape.r = { -8, -8, 16, 16 };
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

    for(unsigned layer = 0; layer < t.layers; layer++) {
		for(unsigned y = 0; y < t.rows; y++) {
            Engine::log("\n");
			for(unsigned x = 0; x < t.columns; x++) {
                unsigned tile = t.tiles[Tiling::tilemap_index(t, layer, x, y)];
                Engine::log("  %d", tile);
            }
        }
    }
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

void system_physics() {
    for(unsigned i = 0; i < box_n; i++) {
        Position &p = boxes[i].position;
        Velocity &v = boxes[i].velocity;
        p.p += v.v;
        v.v *= settings.movement_drag;
    }
}

TileMap tile_map;
void tile_collisions_load() {
    Tiling::tilemap_make(tile_map, 1, 20, 12, 16, 1);
    tilemap_set_collisions(tile_map);
    spawn_box();
}

void tile_collisions_update() {
    system_input();
    system_velocity();
    system_physics();
}

void tile_collisions_render() {
    auto &t = tile_map;
    int x_start = (gw / 2) - (t.columns * t.tile_size / 2);
    int y_start = (gh / 2) - (t.rows * t.tile_size / 2);

    for(unsigned layer = 0; layer < t.layers; layer++) {        
		for(unsigned y = 0; y < t.rows; y++) {
			for(unsigned x = 0; x < t.columns; x++) {
				unsigned tile = t.tiles[Tiling::tilemap_index(t, layer, x, y)];
                if(tile == 1) {
                    draw_g_rectangle_filled_RGBA(x_start + x * t.tile_size, y_start + y * t.tile_size, t.tile_size, t.tile_size, 0, 200, 0, 255);
                } else if(tile == 0) {
                    draw_g_rectangle_filled_RGBA(x_start + x * t.tile_size, y_start + y * t.tile_size, t.tile_size, t.tile_size, 200, 20, 0, 255);
                } else {
                    
                }
			}
		}
	}

    for(unsigned i = 0; i < box_n; i++) {
        draw_g_rectangle_filled_RGBA(boxes[i].position.p.x + boxes[i].collision_shape.r.x, boxes[i].position.p.y + boxes[i].collision_shape.r.y,
            boxes[i].collision_shape.r.w, boxes[i].collision_shape.r.h, 255, 255, 255, 255);
    }
}