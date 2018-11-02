
#include "engine.h"
#include "renderer.h"



//// Tilemap collision expansions
/////////////////////////////////
/////////////////////////////////

struct CollisionShape {
    Rectangle r;
};

struct CollisionSolidBody {
    int x, y;
    bool face_left = false;
    bool face_right = false;
    bool face_top = false;
    bool face_bottom = false;
    Rectangle box;
};

struct CollisionMovingBody {
    Vector2 position;
    Vector2 velocity;
    CollisionShape shape;
    Rectangle box;
};

struct CollisionPhaseData {
    CollisionSolidBody overlapped_solids[64];
    unsigned overlapped_solid_n = 0;
};

std::vector<CollisionSolidBody> debug_overlapped;
std::vector<CollisionSolidBody> debug_collided_solids;

void collision_tilemap_broad_phase(CollisionPhaseData &phase_data, CollisionMovingBody &data, const TileMap &map, const unsigned layer, const unsigned solid);
void collision_tilemap_resolution(CollisionPhaseData &phase_data, CollisionMovingBody &data);
float collision_resolve_x(CollisionMovingBody &data, CollisionSolidBody &solid_body);
float collision_resolve_y(CollisionMovingBody &data, CollisionSolidBody &solid_body);

void collision_init_body(CollisionMovingBody &data, Vector2 velocity, Vector2 position, CollisionShape shape) {
    data.velocity = velocity;
    data.position = position;
    data.shape = shape;
}

void collision_tile_map(CollisionMovingBody &data, const TileMap &map, const unsigned layer, const unsigned solid) {
    CollisionPhaseData phase_data;
    collision_tilemap_broad_phase(phase_data, data, map, layer, solid);

    if(phase_data.overlapped_solid_n > 0) {
        collision_tilemap_resolution(phase_data, data);
    }
}

void collision_tilemap_broad_phase(CollisionPhaseData &phase_data, CollisionMovingBody &data, const TileMap &map, const unsigned layer, const unsigned solid) {
    float left = data.position.x + data.shape.r.x;
    float top = data.position.y + data.shape.r.y;
    float right = left + data.shape.r.w;
    float bottom = top + data.shape.r.h;

    ASSERT_WITH_MSG(left >= 0 && top >= 0, "Left or right is less than zero when finding overlapped tiles!");

    unsigned leftTile = (unsigned)(left / map.tile_size);
    unsigned topTile = (unsigned)(top / map.tile_size);
    unsigned rightTile = (unsigned) Math::ceiling(right / map.tile_size) - 1;
    unsigned bottomTile = (unsigned) Math::ceiling((bottom / map.tile_size)) - 1;
    
    for (unsigned tile_y = topTile; tile_y <= bottomTile; ++tile_y){
        for (unsigned tile_x = leftTile; tile_x <= rightTile; ++tile_x){
            unsigned tile = map.tiles[Tiling::tilemap_index(map, layer, tile_x, tile_y)];

            CollisionSolidBody t;
            t.x = tile_x;
            t.y = tile_y;
            debug_overlapped.push_back(t);

            if(tile == solid) {
                t.box = { 
                    (int)(tile_x * map.tile_size), 
                    (int)(tile_y * map.tile_size), 
                    (int)map.tile_size, 
                    (int)map.tile_size 
                };
                
                // SET FACES ( TOP , LEFT, ... )
                if (tile_x - 1 > 0 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x - 1, tile_y)] != 0) {
                    t.face_left = true;
                }
                if (tile_x + 1 < map.columns 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x + 1, tile_y)] != 0) {
                    t.face_right = true;
                }
                if (tile_y - 1 > 0 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x, tile_y - 1)] != 0) {
                    t.face_top = true;
                }
                if (tile_y + 1 < map.rows 
                    && map.tiles[Tiling::tilemap_index(map, layer, tile_x, tile_y + 1)] != 0) {
                    t.face_bottom = true;
                }
                phase_data.overlapped_solids[phase_data.overlapped_solid_n] = t;
                ++phase_data.overlapped_solid_n;
            }
        }
    }
}

void collision_tilemap_resolution(CollisionPhaseData &phase_data, CollisionMovingBody &data) {
    data.box = {
        (int)(data.position.x + data.shape.r.x), 
        (int)(data.position.y + data.shape.r.y),
        (int)(data.shape.r.w), 
        (int)(data.shape.r.h)
    };

    for(unsigned i = 0; i < phase_data.overlapped_solid_n; i++) {
        CollisionSolidBody solid_body = phase_data.overlapped_solids[i];
        
        if(solid_body.box.intersects(data.box)) {
            debug_collided_solids.push_back(solid_body);

            float minX = 0;
            float minY = 1;

            if (Math::abs_f(data.velocity.x) > Math::abs_f(data.velocity.y)){
                //  Moving faster horizontally, check X axis first
                minX = -1;
            }
            else if (Math::abs_f(data.velocity.x) < Math::abs_f(data.velocity.y)) {
                //  Moving faster vertically, check Y axis first
                minY = -1;
            }

            if (data.velocity.x != 0 && data.velocity.y != 0 
                && (solid_body.face_left || solid_body.face_right) && (solid_body.face_bottom || solid_body.face_top)) {
                minX = (float)Math::min(Math::abs(data.box.x - solid_body.box.right()), Math::abs(data.box.right() - solid_body.box.left()));
                minY = (float)Math::min(Math::abs(data.box.y - solid_body.box.bottom()), Math::abs(data.box.bottom() - solid_body.box.top()));
            }
            
            float ox = 0;
            float oy = 0;
            if (minX < minY) {
                if (solid_body.face_left || solid_body.face_right) {
                    ox = collision_resolve_x(data, solid_body);
                    //  That's horizontal done, check if we still intersects? If not then we can return now
                    if (ox != 0 && !solid_body.box.intersects(data.box)) {
                        continue;
                    }
                }

                if (solid_body.face_top || solid_body.face_bottom) {
                    oy = collision_resolve_y(data, solid_body);
                }
            } else {
                if (solid_body.face_top || solid_body.face_bottom) {
                    oy = collision_resolve_y(data, solid_body);
                    //  That's vertical done, check if we still intersects? If not then we can return now
                    if (oy != 0 && !solid_body.box.intersects(data.box)){
                        continue;
                    }
                }

                if (solid_body.face_left || solid_body.face_right) {
                    ox = collision_resolve_x(data, solid_body);
                }
            }
        }
    }
}

float collision_resolve_y(CollisionMovingBody &data, CollisionSolidBody &solid_body) {
    float oy = 0.0f;
    if (data.velocity.y < 0 && solid_body.face_bottom && data.box.y < solid_body.box.bottom()){
        oy = (float)data.box.y - solid_body.box.bottom();
    } else if (data.velocity.y > 0 && solid_body.face_top && data.box.bottom() > solid_body.box.top()){
        oy = (float)data.box.bottom() - solid_body.box.top();
    }
    if (oy != 0){
        data.position = Vector2(data.position.x, data.position.y - oy);
        data.velocity = Vector2(data.velocity.x, 0);
    }
    return oy;
}

float collision_resolve_x(CollisionMovingBody &data, CollisionSolidBody &solid_body) {
    float ox = 0.0f;
    if (data.velocity.x < 0 && solid_body.face_right && data.box.x < solid_body.box.right()){
        ox = (float)data.position.x - solid_body.box.right();
    } else if (data.velocity.x > 0 && solid_body.face_left && data.box.right() > solid_body.box.left()) {
        ox = (float)data.box.right() - solid_body.box.left();
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

struct BoxMan {
    PlayerInput input;
    Velocity velocity;
    Position position;
    CollisionShape collision_shape;
};

struct Settings {
    // With drag
    //float player_move_speed = 0.5f;
    float movement_drag = 0.90f;

    // Without
    float player_move_speed = 5.0f;
    float player_max_speed = 1.0f;
    
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

        
        // Get mouse direction
        // and mouse click
        /*
        pi.fire_cooldown = Math::max(0.0f, pi.fire_cooldown - Time::deltaTime);
	    if(Input::key_down(key_map.fire)) {
		    pi.fire_x = pi.fire_y = 1;

            // DIRECTION TO MOUSE
            // Maybe this should be in some fire system instead and just let 
            // the input system get that we are going to fire

            float distance = Math::distance_f(s.x, s.y, target.x, target.y);
            if(distance > 30) {
                auto rotation = Math::rads_between_f(s.x, s.y, target.x, target.y);
                s.angle = rotation * Math::RAD_TO_DEGREE;
                
                float direction_x = cos(rotation);
                float direction_y = sin(rotation);
                s.velocity_x += direction_x * config.acceleration;
                s.velocity_y += direction_y * config.acceleration;


	    }*/
    }
}

void system_velocity() {
    for(unsigned i = 0; i < box_n; i++) {
        PlayerInput &pi = boxes[i].input;
        Velocity &v = boxes[i].velocity;
        v.v = pi.move * settings.player_move_speed;
    }
}

void system_player_shoot() {
    /*
    for(unsigned i = 0; i < box_n; i++) {
        PlayerInput &pi = boxes[i].input;
        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            Event e = { Event::FireBullet };
            ShotSpawnData *d = new ShotSpawnData;
            d->position = position;
            d->rotation.x = direction_x;
            d->rotation.y = direction_y;
            d->time_to_live = config.bullet_time_to_live;
            d->faction = sdata.faction;
            e.data = d;
            queue_event(e);
            pi.fire_cooldown = config.fire_cooldown;
        }
    }*/
}

void system_physics(TileMap &tile_map) {
    for(unsigned i = 0; i < box_n; i++) {
        Position &p = boxes[i].position;
        Velocity &v = boxes[i].velocity;

        p.p += v.v;
        
        debug_overlapped.clear();
        debug_collided_solids.clear();
        
        CollisionMovingBody d;
        collision_init_body(d, v.v, p.p, boxes[i].collision_shape);
        const unsigned collision_layer = 0;
        const unsigned solid = 0;
        collision_tile_map(d, tile_map, collision_layer, solid);
        
        p.p = d.position;
        v.v = d.velocity;
        
        //v.v *= settings.movement_drag;
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
    system_player_shoot();
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

    for(unsigned i = 0; i < debug_collided_solids.size(); i++) {
        draw_g_rectangle_filled_RGBA(debug_collided_solids[i].x * tile_map.tile_size, 
                debug_collided_solids[i].y * tile_map.tile_size, 
                tile_map.tile_size, tile_map.tile_size, 
                120, 120, 120, 255);
        
        // if(debug_overlapped[i].face_bottom) {
        //     draw_g_horizontal_line_RGBA(debug_overlapped[i].x * tile_map.tile_size,
        //     debug_overlapped[i].x * tile_map.tile_size + tile_map.tile_size,
        //     debug_overlapped[i].y * tile_map.tile_size,
        //     255, 0, 0, 255);
        // }
    }

    for(unsigned i = 0; i < box_n; i++) {
        draw_g_rectangle_filled_RGBA((int)boxes[i].position.p.x + boxes[i].collision_shape.r.x, 
            (int)boxes[i].position.p.y + boxes[i].collision_shape.r.y,
            boxes[i].collision_shape.r.w, boxes[i].collision_shape.r.h, 
            255, 255, 255, 255);
    }
}