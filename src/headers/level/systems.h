#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "entities.h"
#include "game_area_controller.h"
#include "game_events.h"

struct InputMapping {
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
	SDL_Scancode fire;
	SDL_Scancode shield;
};

const static InputMapping input_maps[2] = {
	{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_SPACE, SDL_SCANCODE_LSHIFT },
	{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RSHIFT }
};

inline void system_player_get_input(Player &players) {
    bool yes = false;
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        pi.move_x = 0;
        pi.move_y = 0;
        pi.fire_x = 0;
        pi.fire_y = 0;

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

        pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::delta_time);

        if(Input::key_down(key_map.fire)) {
            pi.fire_x = pi.fire_y = 1;
            yes = true;
        }

        if(Input::key_pressed(SDLK_p)) {
            Engine::pause(1.0f);
        }
    }

    if(yes) {
        int a = 4;
        a++;
    }
}

inline void system_player_handle_input(Player &players, GameAreaController *game_ctrl) {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        Velocity &velocity = players.velocity[i];
        Direction &direction = players.direction[i];
        const PlayerConfiguration &player_config = players.config[i];
        const auto &player_position = players.position[i];
        
        // Update rotation based on rotational speed
        direction.angle += pi.move_x * player_config.rotation_speed * Time::delta_time;
        if(direction.angle > 360.0f) {
            direction.angle = 0;
        } else if(direction.angle < 0.0f) {
            direction.angle = 360.0f;
        }
        float rotation = direction.angle / Math::RAD_TO_DEGREE;
        direction.value.x = cos(rotation);
        direction.value.y = sin(rotation);
        
	    velocity.value.x += direction.value.x * pi.move_y * player_config.move_acceleration * Time::delta_time;
	    velocity.value.y += direction.value.y * pi.move_y * player_config.move_acceleration * Time::delta_time;

        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            pi.fire_cooldown = players.weapon[i].fire_cooldown;

            auto projectile_pos = player_position;

            // auto fire_dir = Math::direction(Vector2(Input::mousex, Input::mousey), projectile_pos.value);
            // const Vector2 projectile_direction = fire_dir;
            const Vector2 projectile_direction = direction.value;

            // set the projectile position to be gun_barrel_distance infront of the ship
            projectile_pos.value.x += projectile_direction.x * player_config.gun_barrel_distance;
            projectile_pos.value.y += projectile_direction.y * player_config.gun_barrel_distance;        
            auto muzzle_pos = projectile_pos;

            // Accuracy
            const float accuracy = 8; // how far from initial position it can maximaly spawn
            projectile_pos.value.x += RNG::range_f(-accuracy, accuracy) * projectile_direction.y;
            projectile_pos.value.y += RNG::range_f(-accuracy, accuracy) * projectile_direction.x;

            float projectile_speed = players.weapon[i].projectile_speed;
            Vector2 projectile_velocity = Vector2(projectile_direction.x * projectile_speed, projectile_direction.y * projectile_speed);
            game_ctrl->spawn_player_projectile(projectile_pos.value, projectile_velocity);
            game_ctrl->spawn_muzzle_flash(muzzle_pos, Vector2(player_config.gun_barrel_distance, player_config.gun_barrel_distance), players.entity[i]);
            
            camera_shake(0.1f);

            camera_displace(projectile_direction * player_config.fire_knockback_camera);

            game_ctrl->spawn_smoke(muzzle_pos.value);
            
            // Player knockback
            players.position[i].value.x -= projectile_direction.x * player_config.fire_knockback;
            players.position[i].value.y -= projectile_direction.y * player_config.fire_knockback;

            Sound::queue(game_ctrl->sound_map["player_fire"], 2);
            // auto b = GameEvents::get_event<PlayerFireBullet>();
            // b->test = 666;
            // GameEvents::queue(b);
        }
    }
}

template<typename T>
void move_forward(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        entity_data.position[i].value.x += entity_data.velocity[i].value.x * Time::delta_time;
        entity_data.position[i].value.y += entity_data.velocity[i].value.y * Time::delta_time;
    }
}

template<typename T>
void keep_in_bounds(T &entity_data, Rectangle &bounds) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.position[i].value.x < bounds.x) { 
            entity_data.position[i].value.x = (float)bounds.x; 
        }
        if(entity_data.position[i].value.x > bounds.right()) { 
            entity_data.position[i].value.x = (float)bounds.right(); 
        }
        if(entity_data.position[i].value.y < bounds.y) { 
            entity_data.position[i].value.y = (float)bounds.y; 
        }
        if(entity_data.position[i].value.y > bounds.bottom()) { 
            entity_data.position[i].value.y = (float)bounds.bottom(); 
        }
    } 
}

template<typename T>
void set_last_position(T &entity_data) {
    // Set last position of projectile so we can use that in collision handling etc
    for(int i = 0; i < entity_data.length; ++i) {
        entity_data.position[i].last = entity_data.position[i].value;
    }
}

template<typename T>
void remove_out_of_bounds(T &entity_data, Rectangle &bounds) {
    for(int i = 0; i < entity_data.length; i++) {
        if(!bounds.contains((int)entity_data.position[i].value.x, (int)entity_data.position[i].value.y)) {
            mark_for_deletion(entity_data, entity_data.entity[i]);
        }
    } 
}

template<typename T>
void system_blink_effect(T &entity_data) {
    for(int i = 0; i < entity_data.length; ++i) {
        entity_data.blink[i].timer += Time::delta_time;
        entity_data.blink[i].interval_timer += Time::delta_time;

        if(entity_data.blink[i].timer >= entity_data.blink[i].time_to_live) {
            entity_data.blink[i].timer = 0.0f;
            if(entity_data.blink[i].time_to_live > 0) {
                entity_data.sprite[i].sprite_name = entity_data.blink[i].original_sprite;
            }
            entity_data.blink[i].time_to_live = 0;
            continue;
        }
        
        if(entity_data.blink[i].interval_timer > entity_data.blink[i].interval) {
            entity_data.sprite[i].sprite_name = 
                entity_data.sprite[i].sprite_name == entity_data.blink[i].original_sprite 
                    ? entity_data.blink[i].white_sprite : entity_data.blink[i].original_sprite;
            entity_data.blink[i].interval_timer = 0;
        }
    }
}

inline void system_effects(Effect &effects, Player &players, Target &targets) {
    for(int i = 0; i < effects.length; ++i) {
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
            if(effect.timer == effects.effect[i].modifier_time) {
                effect.modifier(effects, i, effect.modifier_data_s);
            }
        }

        effect.timer += Time::delta_time;
    }
}

inline void system_remove_completed_effects(Effect &effects) {
    for(int i = 0; i < effects.length; ++i) {
        if(effects.effect[i].timer > effects.effect[i].time_to_live) {
            mark_for_deletion(effects, effects.entity[i]);
        }
    }
}

template<typename T>
void system_drag(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        const float drag = entity_data.config[i].drag;
        Velocity &velocity = entity_data.velocity[i];
        velocity.value = velocity.value - velocity.value * drag * Time::delta_time;
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
        const float projectile_radius = (float)entity_first.collision[i].radius;
        const Vector2 &p_last = entity_first.position[i].last;

        for(int j = 0; j < entity_second.length; ++j) {
            const Vector2 &t_pos = entity_second.position[j].value;
            const float t_radius = (float)entity_second.collision[j].radius;
            
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

template<typename T>
void system_camera_follow(const T &entity_data, int i, float distance) {
    if(entity_data.length <= i) {
        return;
    }

    Vector2 position = entity_data.position[i].value;
    const Vector2 &direction = entity_data.direction[i].value;
    position += direction * distance;
    camera_follow(position);
}

template<typename T>
void system_child_sprite_position(ChildSprite &child_sprites, const T &entity_data) {
    for(size_t i = 0; i < child_sprites.length; ++i) {
        if(entity_data.contains(child_sprites.parent[i])) {
            const auto handle = entity_data.get_handle(child_sprites.parent[i]);
            const auto direction = child_sprites.direction[i].value.length() > 0 
                ? (entity_data.direction[handle.i].value * child_sprites.direction[i].value)
                : Vector2::One;
            child_sprites.position[i].value = entity_data.position[handle.i].value + child_sprites.local_position[i] 
                * direction;
            child_sprites.sprite[i].rotation = entity_data.sprite[handle.i].rotation;
        } else {
            // Remove it
            child_sprites.remove(i);
        }
    }
}

template<typename T>
void system_animation_ping_pong(T &entity_data) {
    for(size_t i = 0; i < entity_data.length; ++i) {
        auto &animation = entity_data.animation[i];
        if(!animation.enabled) {
            continue;
        }

        animation.timer += Time::delta_time;
		if(animation.timer > animation.duration * 2) {
			animation.timer = 0;
        }
		
		float time = animation.timer > animation.duration ? animation.duration - (animation.timer - animation.duration) : animation.timer;
		animation.value = Math::interpolate(animation.start, animation.end, time/animation.duration, animation.ease);

        entity_data.sprite[i].h = (int)animation.value;
    }
}

template<typename T>
void system_child_sprite_exhaust(T &entity_data, ChildSprite &child_sprites) {
    for(int i = 0; i < entity_data.length; i++) {
        PlayerInput &pi = entity_data.input[i];
        const PlayerConfiguration &player_config = entity_data.config[i];
        int exhaust_id = entity_data.get_child_sprite_index(player_config.exhaust_id);

        auto &exhaust_animation = entity_data.child_sprites.animation[exhaust_id];
        auto &local_position = entity_data.child_sprites.local_position[exhaust_id];
     
        if(pi.move_y > 0) {
            local_position = Vector2(-player_config.gun_barrel_distance, -player_config.gun_barrel_distance);
            exhaust_animation.start = 24;
            exhaust_animation.end = 28;
        }
        else {
            local_position = Vector2(-player_config.gun_barrel_distance + 3, -player_config.gun_barrel_distance + 3);
            exhaust_animation.start = 4;
            exhaust_animation.end = 6;
        }
    }
}

inline void system_player_ship_animate(Player &players) {
    system_child_sprite_position(players.child_sprites, players);
    system_child_sprite_exhaust(players, players.child_sprites);
    system_animation_ping_pong(players.child_sprites);

    for(int i = 0; i < players.length; i++) {
        if(players.input[i].move_x > 0) {
            players.sprite[i].sprite_name = "player_turn_right";
        } else if(players.input[i].move_x < 0) {
            players.sprite[i].sprite_name = "player_turn_left";
        } else {
            players.sprite[i].sprite_name = "player_1";
        }
    }
}

inline void system_target_ship_animate(Target &targets) {
    system_child_sprite_position(targets.child_sprites, targets);
}

template<typename T>
void system_invulnerability(T &entity_data, const float dt) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.health[i].invulnerability_timer > 0.0f) {
            entity_data.health[i].invulnerability_timer -= dt;
            // Engine::logn("invul: %.2f", entity_data.health[i].invulnerability_timer);
        }
    }
}

template<typename T>
void system_remove_no_health_left(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.health[i].hp <= 0) {
            mark_for_deletion(entity_data, entity_data.entity[i]);
        }
    }
}

template<typename AI, typename Enemy>
void system_ai_input(AI &entity_data, Enemy &entity_search_targets, Projectile &projectiles) {
    for(int i = 0; i < entity_data.length; i++) {
        entity_data.ai[i].fire_cooldown = Math::max_f(0.0f, entity_data.ai[i].fire_cooldown - Time::delta_time);

        if(entity_data.ai[i].fire_cooldown > 0.0f) {
            continue;
        }

        for(int t_i = 0; t_i < entity_search_targets.length; t_i++) {
            const Vector2 &ai_position = entity_data.position[i].value;
            const Vector2 &target_position = get_position(entity_search_targets, entity_search_targets.entity[t_i]).value;
            if(entity_data.ai[i].search_area > Math::distance_v(ai_position, target_position)) {
                entity_data.ai[i].fire_cooldown = entity_data.weapon[i].fire_cooldown;;

                const Vector2 direction = Math::direction(target_position, ai_position);
                const Vector2 projectile_position = ai_position;
                
                Vector2 projectile_velocity = direction * entity_data.weapon[i].projectile_speed;                
                projectiles.queue_projectile(projectile_position, projectile_velocity);
                continue; // only fire at one target
            }
        }
    }
}

template<typename T>
void system_remove_deleted(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.life_time[i].marked_for_deletion) {
            entity_data.life_time[i].marked_for_deletion = false;
            entity_data.remove(entity_data.entity[i]);
        }
    }
}

// Player is dealt damage
inline void on_deal_damage(Projectile &projectile, Player &p, const CollisionPair &entities, GameAreaController *game_ctrl) {
    int amount_dealt = deal_damage(projectile, entities.first, p, entities.second);

    auto &health = get_health(p, entities.second);
    if(health.hp <= 0) {
        auto &second_pos = get_position(p, entities.second);
        game_ctrl->spawn_explosion(second_pos.value, 10, 10);

        // TODO: Trigger some kind of death thing so we know game is over
    } else if(amount_dealt > 0) {
        // play hit sound
        // Sound::queue(test_sound_id, 2);

        camera_shake(0.1f);

        // 29 frames because that is so cool
        float invulnerability_time = 29 * Time::delta_time_fixed;
        set_invulnerable(health, invulnerability_time);
        blink_sprite(p, entities.second, invulnerability_time, 5 * Time::delta_time_fixed);
    } else {
        Engine::logn("CASE NOT IMPLEMENTED -> no damage dealt on player");
        // Do we need to handle this case?
    }
}

// Target is dealt damage
inline void on_deal_damage(Projectile &projectile, Target &t, const CollisionPair &entities, GameAreaController *game_ctrl) {
    // Knockback
    auto &damage = get_damage(projectile, entities.first);
    auto &velocity = get_velocity(projectile, entities.first);
    auto &second_pos = get_position(t, entities.second);
    Vector2 dir = Math::normalize(Vector2(velocity.value.x, velocity.value.y));
    second_pos.value.x += dir.x * damage.force;
    second_pos.value.y += dir.y * damage.force;

    int amount_dealt = deal_damage(projectile, entities.first, t, entities.second);
    
    Engine::pause(0.03f);

    // Emit hit particles
    const auto &pos = get_position(projectile, entities.first);
    float angle = Math::degrees_between_v(pos.last, entities.collision_point);
    game_ctrl->spawn_hit_effect(entities.collision_point, angle);
    
    auto &health = get_health(t, entities.second);
    if(health.hp <= 0) {
        // play explosion sound / death sound
        // OR DO THIS IN SPAWN EXPLOSION METHOD
        // Sound::queue(test_sound_id, 2);

        camera_shake(0.1f);
        
        auto &p = get_position(t, entities.second);
        game_ctrl->spawn_explosion(p.value, 10, 10);
    } else if(amount_dealt > 0) {
        // play hit sound
        // Sound::queue(test_sound_id, 2);
        
        // 29 frames because that is so cool
        float invulnerability_time = 29 * Time::delta_time_fixed;
        set_invulnerable(health, invulnerability_time);
        blink_sprite(t, entities.second, invulnerability_time, 5 * Time::delta_time_fixed);
    } else {
        Engine::logn("CASE NOT IMPLEMENTED -> no damage dealt");
        // Do we need to handle this case?
    }
}

template<typename First, typename Second>
void system_collision_resolution(CollisionPairs &collision_pairs, First &entity_first, Second &entity_second, GameAreaController *game_ctrl) {
    collision_pairs.sort_by_distance();

    // This set will contain all collisions that we have handled
    // Since first in this instance is projectile and the list is sorted by distance
    // we only care about the collision with the shortest distance in this implementation
    std::unordered_set<ECS::EntityId> handled_collisions;
    for(int i = 0; i < collision_pairs.count; ++i) {
        if(handled_collisions.find(collision_pairs[i].first.id) != handled_collisions.end()) {
            continue;
        }
        handled_collisions.insert(collision_pairs[i].first.id);
        debug_config.last_collision_point = collision_pairs[i].collision_point;

        // All of this could be an event
        // or it could be many events but the less the simpler right
        // but for now it's just easier to call functions directly since we don't have
        // anything else listening to things
        mark_for_deletion(entity_first, collision_pairs[i].first);
        // on_hit(entity_first, entity_second, collision_pairs[i]);
        if(is_invulnerable(entity_second, collision_pairs[i].second)) {
            continue;
        }
        on_deal_damage(entity_first, entity_second, collision_pairs[i], game_ctrl);
        // ----
    }
}

#endif