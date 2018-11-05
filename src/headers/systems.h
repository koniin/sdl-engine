#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "entities.h"

struct InputMapping {
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
	SDL_Scancode fire;
	SDL_Scancode shield;
};

InputMapping input_maps[2] = {
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_SPACE, SDL_SCANCODE_LSHIFT },
	{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RSHIFT }
};

std::vector<ECS::Entity> entities_to_destroy;
void queue_remove_entity(ECS::Entity entity) {
    entities_to_destroy.push_back(entity);
}

void system_player_get_input(Player players) {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        pi.move_x = 0;
        pi.move_y = 0;
        pi.fire_x = 0;
        pi.fire_y = 0;
        pi.shield = false;

        InputMapping key_map = input_maps[i];

        if(Input::key_down(key_map.up)) {
            pi.move_y = 1;
        } else if(Input::key_down(key_map.down)) {
            pi.move_y = -1;
        } 
        
        if(Input::key_down(key_map.left)) {
            pi.move_x = -1;
        } else if(Input::key_down(key_map.right)) {
            pi.move_x = 1;
        }

        pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::deltaTime);
        if(Input::key_down(key_map.fire)) {
            pi.fire_x = pi.fire_y = 1;
        }

        if(Input::key_down(key_map.shield)) {
            pi.shield = true;
        }

        if(Input::key_pressed(SDLK_p)) {
            Engine::pause(1.0f);
        }
    }
}

template<typename T>
void move_forward(T &entityData) {
    for(int i = 0; i < entityData.length; i++) {
        entityData.position[i].value.x += entityData.velocity[i].value.x * Time::deltaTime;
        entityData.position[i].value.y += entityData.velocity[i].value.y * Time::deltaTime;
    }
}

template<typename T>
void keep_in_bounds(T &entityData, Rectangle &bounds) {
    for(int i = 0; i < entityData.length; i++) {
        if(entityData.position[i].value.x < bounds.x) { 
            entityData.position[i].value.x = (float)bounds.x; 
        }
        if(entityData.position[i].value.x > bounds.right()) { 
            entityData.position[i].value.x = (float)bounds.right(); 
        }
        if(entityData.position[i].value.y < bounds.y) { 
            entityData.position[i].value.y = (float)bounds.y; 
        }
        if(entityData.position[i].value.y > bounds.bottom()) { 
            entityData.position[i].value.y = (float)bounds.bottom(); 
        }
    } 
}

template<typename T>
void remove_out_of_bounds(T &entityData, Rectangle &bounds) {
    for(int i = 0; i < entityData.length; i++) {
        if(!bounds.contains((int)entityData.position[i].value.x, (int)entityData.position[i].value.y)) {
            queue_remove_entity(entityData.entity[i]);
        }
    } 
}

template<typename T>
void system_blink_effect(T &entity_data) {
    for(int i = 0; i < entity_data.length; ++i) {
        ++entity_data.blink[i].frame_counter;
        if(entity_data.blink[i].frame_counter > entity_data.blink[i].frames_to_live) {
            entity_data.blink[i].frame_counter = 0;
            if(entity_data.blink[i].frames_to_live > 0) {
                entity_data.sprite[i].sprite_sheet_index = entity_data.blink[i].original_sheet;
            }
            entity_data.blink[i].frames_to_live = 0;
            continue;
        }
        
        if(!(entity_data.blink[i].frame_counter % entity_data.blink[i].interval)) {
            entity_data.sprite[i].sprite_sheet_index = 
                entity_data.sprite[i].sprite_sheet_index == entity_data.blink[i].original_sheet 
                    ? entity_data.blink[i].white_sheet : entity_data.blink[i].original_sheet;
        }
    }
}

void system_effects(Effect effects, Player players, Target targets) {
    for(int i = 0; i < effects.length; ++i) {
        if(effects.effect[i].frame_counter > effects.effect[i].frames_to_live) {
            queue_remove_entity(effects.entity[i]);
            continue;
        }

        if(effects.effect[i].has_target) {
            if(players.contains(effects.effect[i].follow)) {
                auto handle = players.get_handle(effects.effect[i].follow);
                Position pos = players.position[handle.i];
                const Direction &direction = players.direction[handle.i];
                pos.value.x += direction.value.x * effects.effect[i].local_position.x;
                pos.value.y += direction.value.y * effects.effect[i].local_position.y;
                effects.position[i] = pos;
            }
            if(targets.contains(effects.effect[i].follow)) {
                Engine::logn("following a target - not implemented");
            }
        }
        
        auto &effect = effects.effect[i];
        if(effects.effect[i].modifier_enabled) {
            if(effect.frame_counter == effects.effect[i].modifier_frame) {
                effect.modifier(effects, i, effect.modifier_data_s);
            }
        }
        
        effects.effect[i].frame_counter++;
    }
}

template<typename T>
void system_drag(T &entity_data, float drag) {
    for(int i = 0; i < entity_data.length; i++) {
        Velocity &velocity = entity_data.velocity[i];
	    velocity.value.x = velocity.value.x - velocity.value.x * drag;
	    velocity.value.y = velocity.value.y - velocity.value.y * drag;
    }
}

struct CollisionPair {
    ECS::Entity first;
    ECS::Entity second;
    float distance;
    Vector2 collision_point;

    bool operator<( const CollisionPair& rhs ) const { 
        return distance < rhs.distance; 
    }
};

struct CollisionPairs {
    std::vector<CollisionPair> collisions;
    int count = 0;

    inline CollisionPair operator [](size_t i) const { return collisions[i]; }
    inline CollisionPair & operator [](size_t i) { return collisions[i]; }

    void allocate(size_t size) {
        collisions.reserve(size);
    }

    void sort_by_distance() {
        std::sort(collisions.begin(), collisions.end());
    }

    void push(ECS::Entity first, ECS::Entity second, float distance, Vector2 collision_point) {
        collisions.push_back({ first, second, distance, collision_point });
        count++;
    }

    void clear() {
        count = 0;
        collisions.clear();
    }
};

template<typename First, typename Second>
void system_collisions(CollisionPairs &collision_pairs, First &entity_first, Second &entity_second) {
    for(int i = 0; i < entity_first.length; ++i) {
        const Vector2 &p_pos = entity_first.position[i].value;
        const float projectile_radius = (float)entity_first.radius;
        
        for(int j = 0; j < entity_second.length; ++j) {
            const Vector2 &t_pos = entity_second.position[j].value;
            const float t_radius = (float)entity_second.radius;
            const Vector2 &p_last = entity_first.position[i].last;

            // Distance from projectiles last position and targets new position
            // should get the closest target in projectiles path
            float dist = Math::distance_v(p_last, t_pos);
            if(Math::intersect_circles(p_pos.x, p_pos.y, projectile_radius, 
                    t_pos.x, t_pos.y, t_radius)) {
                // Collision point is the point on the target circle 
                // that is on the edge in the direction of the projectiles 
                // reverse velocity
                Engine::logn("circle intersect");
                Vector2 collision_point = t_pos + (t_radius * -entity_first.velocity[i].value.normal());
                collision_pairs.push(entity_first.entity[i], entity_second.entity[j], dist, collision_point);
                continue;
            }
            
            Vector2 entry_point;
            int result = Intersects::line_circle_entry(p_last, p_pos, t_pos, t_radius, entry_point);
            if(result == 1 || result == 2) {
                Vector2 collision_point = t_pos + (t_radius * -entity_first.velocity[i].value.normal());
                collision_pairs.push(entity_first.entity[i], entity_second.entity[j], dist, collision_point);
                Engine::logn("line intersect");
            }
        }
    }
}

#endif