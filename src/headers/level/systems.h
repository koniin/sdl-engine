#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "game_input_wrapper.h"
#include "entities.h"
#include "game_area_controller.h"
#include "game_events.h"

inline void system_player_get_input(Player &players) {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        pi.move.x = pi.move.y = 0;
        pi.fire_x = 0;
        pi.fire_y = 0;

        GInput::direction(pi.move);

        pi.fire_cooldown = Math::max_f(0.0f, pi.fire_cooldown - Time::delta_time);

        if(GInput::down(GInput::Fire)) {
            pi.fire_x = pi.fire_y = 1;
        }

        if( GInput::pressed(GInput::Pause)) {
            Engine::pause(1.0f);
        }
    }
}

inline void system_player_handle_input(Player &players, GameAreaController *game_ctrl) {
    for(int i = 0; i < players.length; i++) {
        PlayerInput &pi = players.input[i];
        Velocity &velocity = players.velocity[i];
        Direction &direction = players.direction[i];
        const PlayerConfiguration &player_config = players.config[i];
        
        // Update rotation based on rotational speed
        direction.angle += pi.move.x * player_config.rotation_speed * Time::delta_time;
        if(direction.angle > 360.0f) {
            direction.angle = 0;
        } else if(direction.angle < 0.0f) {
            direction.angle = 360.0f;
        }

        direction.value = Math::direction_from_angle(direction.angle);
        
        if(pi.move.y >= 0) {
            velocity.value += direction.value * pi.move.y * player_config.move_acceleration * Time::delta_time;
        } else {
            velocity.value = velocity.value - velocity.value * player_config.brake_speed * Time::delta_time;
        }
        
        float max = player_config.max_velocity;
        float min = -player_config.max_velocity;
        Vector2 v = velocity.value;
        if (v.length() > max) {
            v = v.normal() * max;
        } else if (v.length() < min) {
            v = v.normal() * min;
        }
        velocity.value = v;
        
        
	    // velocity.value.y += direction.value.y * pi.move_y * player_config.move_acceleration * Time::delta_time;

        if(pi.fire_cooldown <= 0.0f && Math::length_vector_f(pi.fire_x, pi.fire_y) > 0.5f) {
            // From config (depends on rendering size)
            const float gun_barrel_distance = player_config.gun_barrel_distance;
            float original_angle = direction.angle;
            // set the projectile start position to be gun_barrel_distance infront of the ship
            auto gun_exit_position = players.position[i].value + Math::direction_from_angle(original_angle) * gun_barrel_distance;
            
            Ammunition &ammo = players.ammo[i];
            auto fire_result = game_ctrl->player_projectile_fire(ammo.value, original_angle, gun_exit_position);

            pi.fire_cooldown = fire_result.fire_cooldown;

            if(!fire_result.did_fire) {
                Engine::logn("Implement some kind of out of ammo sound etc");
                return;
            }

            // this is for all projectiles
            // ---------------------------------
            ammo.value = Math::max_i(ammo.value - fire_result.ammo_used, 0);
            
            // Muzzle flash
            game_ctrl->spawn_muzzle_flash_effect(gun_exit_position, Vector2(gun_barrel_distance, gun_barrel_distance), players.entity[i]);
            // Camera
            auto projectile_direction = Math::direction_from_angle(original_angle);
            camera_shake(0.1f);
            camera_displace(projectile_direction * player_config.fire_knockback_camera);
            // Player knockback
            players.position[i].value -= projectile_direction * fire_result.knockback;
            // Sound
            Sound::queue(game_ctrl->sound_map[fire_result.sound_name], 2);
            // ---------------------------------
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
void system_drag(T &entity_data, const float drag) {
    for(int i = 0; i < entity_data.length; i++) {
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
                // Engine::logn("circle intersect");
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
     
        if(pi.move.y > 0) {
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
        if(players.input[i].move.x > 0) {
            players.sprite[i].sprite_name = "player_turn_right";
        } else if(players.input[i].move.x < 0) {
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
void system_ai_input(AI &entity_data, Enemy &entity_search_targets, Projectile &projectiles, GameAreaController *ga_ctrl) {
    for(int i = 0; i < entity_data.length; i++) {
        entity_data.ai[i].fire_cooldown = Math::max_f(0.0f, entity_data.ai[i].fire_cooldown - Time::delta_time);

        if(entity_data.ai[i].fire_cooldown > 0.0f) {
            continue;
        }

        for(int t_i = 0; t_i < entity_search_targets.length; t_i++) {
            const Vector2 &ai_position = entity_data.position[i].value;
            const Vector2 &target_position = get_position(entity_search_targets, entity_search_targets.entity[t_i]).value;
            if(entity_data.ai[i].fire_range > Math::distance_v(ai_position, target_position)) {
                entity_data.ai[i].fire_cooldown = entity_data.weapon[i].fire_cooldown;;

                // const Vector2 direction = Math::direction(target_position, ai_position);
                float angle = Math::degrees_between_v(ai_position, target_position);
                
                //Vector2 projectile_velocity = direction * entity_data.weapon[i].projectile_speed;                

                ProjectileSpawn p = ProjectileSpawn(ai_position, angle, entity_data.weapon[i].projectile_speed, 1, 8, 1.0f, 0, 0);
                ga_ctrl->target_projectile_fire(p);
                continue; // only fire at one target
            }
        }
    }
}

template<typename AIEntity, typename EnemyEntity>
void system_ai_movement(AIEntity &entity_data, EnemyEntity &entity_search_targets, const Rectangle &world_bounds) {
    int buffer_area = 32;
    Rectangle inflated_world_bounds = Rectangle(
        buffer_area, 
        buffer_area, 
        world_bounds.w - buffer_area * 2, 
        world_bounds.h - buffer_area * 2
    );

    for(int i = 0; i < entity_data.length; i++) {
        // entity_data.ai[i].fire_cooldown = Math::max_f(0.0f, entity_data.ai[i].fire_cooldown - Time::delta_time);

        // if(entity_data.ai[i].fire_cooldown > 0.0f) {
        //     continue;
        // }

        const Vector2 &ai_position = entity_data.position[i].value;
        AIComponent &ai = entity_data.ai[i];

        for(int t_i = 0; t_i < entity_search_targets.length; t_i++) {
            const Vector2 &target_position = get_position(entity_search_targets, entity_search_targets.entity[t_i]).value;
            const float distance_to_player = Math::distance_v(ai_position, target_position);
            if(ai.activated || ai.engagement_range > distance_to_player) {
                ai.activated = true;

                if(distance_to_player > ai.target_min_range) {
                    Vector2 dist = target_position - ai_position;
    
                    if(!inflated_world_bounds.contains(ai_position.to_point())) {
                        dist = inflated_world_bounds.center() - ai_position;
                    }

                    entity_data.velocity[i].value += Math::scale_to(dist, ai.acceleration);
                    
                    // if (Velocity != Vector2::Zero)
                    //     Entity.Transform.Rotation = Velocity.ToAngle();
                } else {
                    static int times_run;
                    int times_to_run = 6;
                    static bool initialized = false;
                    static float direction = 0;
                    if(!initialized) {
                        initialized = true;
                        direction = RNG::range_f(0, Math::TwoPi);
                    }

                    if(times_run > times_to_run) {
                        times_run = 0;
                        direction += RNG::range_f(-0.1f, 0.1f);
                        direction = Math::wrap_angle(direction);
                    }
                    times_run++;
                    
                    entity_data.velocity[i].value += Math::polar_coords_to_vector(direction, 5.4f);

                    if(!inflated_world_bounds.contains(ai_position.to_point())) {
                        auto ang = Math::angle_from_direction(inflated_world_bounds.center() - ai_position);
                        direction = ang + RNG::range_f(-Math::PiOver2, Math::PiOver2);
                    }


                    // move x times in a general direction
                    // then get a new random direction to move in
                    // when close to bounds move away

                    // float direction = RNG.Range(0, MathHelper.TwoPi);
                    // while (true) {
                    //     direction += RNG.Range(-0.1f, 0.1f);
                    //     direction = MathHelper.WrapAngle(direction);

                    //     for (int i = 0; i < 6; i++) {
                    //         Velocity += Mathf.FromPolar(direction, 0.4f);
                    //         Entity.Transform.Rotation -= 0.05f;

                            
                    //         var bounds = EngineCore.VirtualSize;
                    //         bounds.Inflate(-image.Width, -image.Height);
                            
                    //         // if the enemy is outside the bounds, make it move away from the edge
                    //         if (!bounds.Contains(Position.ToPoint()))
                    //             direction = (EngineCore.VirtualSize.Size.ToVector2() / 2 - Position).ToAngle() + RNG.Range(-MathHelper.PiOver2, MathHelper.PiOver2);

                    //         yield return 0;
                    //     }
                    // }
                }
                
                
                Direction &direction = entity_data.direction[i];
                direction.angle = Math::angle_from_direction(entity_data.velocity[i].value);
                if(direction.angle < 0) {
                    direction.angle = 360 - (-direction.angle);
                } else if(direction.angle > 360.0f) {
                    direction.angle = direction.angle - 360.0f;
                }
                
                continue;
            }
        }
    }
}

template<typename T>
void system_update_life_time(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.life_time[i].ttl > 0.0f) {
            entity_data.life_time[i].time += Time::delta_time;
            if(!entity_data.life_time[i].marked_for_deletion && entity_data.life_time[i].time >= entity_data.life_time[i].ttl) {
                entity_data.life_time[i].marked_for_deletion = true;
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
    // Remove projectile
    mark_for_deletion(projectile, entities.first);

    if(is_invulnerable(p, entities.second)) {
        return;
    }
    
    int amount_dealt = deal_damage(projectile, entities.first, p, entities.second);

    auto &health = get_health(p, entities.second);
    if(health.hp <= 0) {
        auto &second_pos = get_position(p, entities.second);
        game_ctrl->spawn_explosion_effect(second_pos.value, 10, 10);

        // TODO: Trigger some kind of death thing so we know game is over
    } else if(amount_dealt > 0) {
        // play hit sound
        // Sound::queue(test_sound_id, 2);

        camera_shake(0.1f);

        // 20 frames because that is so cool
        float invulnerability_time = 20 * Time::delta_time_fixed;
        set_invulnerable(health, invulnerability_time);
        blink_sprite(p, entities.second, invulnerability_time, 5 * Time::delta_time_fixed);
    } else {
        Engine::logn("Shield hit or other type of invulnerability");
        // Do we need to handle this case?
    }
}

// Target is dealt damage
inline void on_deal_damage(Projectile &projectile, Target &t, const CollisionPair &entities, GameAreaController *game_ctrl) {
    bool invulnerable = is_invulnerable(t, entities.second);
    auto handle = projectile.get_handle(entities.first);
    if(!invulnerable && projectile.is_valid(handle)) {
        if(++projectile.pierce[handle.i].count > projectile.pierce[handle.i].limit) {
            mark_for_deletion(projectile, entities.first);
        }
        
        game_ctrl->player_projectile_hit(entities.first, entities.second, entities.collision_point);
    } else {
        return;
    }

    // Knockback
    auto &damage = get_damage(projectile, entities.first);
    auto &velocity = get_velocity(projectile, entities.first);
    auto &second_pos = get_position(t, entities.second);
    if(velocity.value != Vector2::Zero) {
        Vector2 dir = Math::normalize(Vector2(velocity.value.x, velocity.value.y));
        second_pos.value.x += dir.x * damage.force;
        second_pos.value.y += dir.y * damage.force;
    }
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

        auto target_killed_event = GameEvents::get_event<TargetKilled>();
        target_killed_event->test = 44;
        GameEvents::queue(target_killed_event);

        game_ctrl->spawn_drop(pos.value);

        camera_shake(0.1f);
        
        auto &p = get_position(t, entities.second);
        game_ctrl->spawn_explosion_effect(p.value, 10, 10);
    } else if(amount_dealt > 0) {
        // play hit sound
        // Sound::queue(test_sound_id, 2);
        
        Engine::logn("hp %d", health.hp);

        // 6 frames because that is so cool
        float invulnerability_time = 6 * Time::delta_time_fixed;
        set_invulnerable(health, invulnerability_time);
        blink_sprite(t, entities.second, invulnerability_time, 3 * Time::delta_time_fixed);
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

        on_deal_damage(entity_first, entity_second, collision_pairs[i], game_ctrl);
        // ----
    }
}

template<typename FirstDrop, typename Second>
void system_collision_resolution_drops(CollisionPairs &collision_pairs, FirstDrop &entity_first, Second &entity_second, GameAreaController *game_ctrl) {
    for(int i = 0; i < collision_pairs.count; ++i) {
        Engine::logn("Got a drop!");
        mark_for_deletion(entity_first, collision_pairs[i].first);
    }
}

template<typename T>
void system_ammo_recharge(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        entity_data.ammo[i].timer += Time::delta_time;
        if(entity_data.ammo[i].timer > entity_data.ammo[i].recharge_time) {
            entity_data.ammo[i].value = Math::min_i(entity_data.ammo[i].max, entity_data.ammo[i].value + entity_data.ammo[i].recharge);
            entity_data.ammo[i].timer = 0.0f;
        }       
    }
}

template<typename T>
void system_shield_recharge(T &entity_data) {
    for(int i = 0; i < entity_data.length; i++) {
        entity_data.shield[i].timer += Time::delta_time;
        if(entity_data.shield[i].timer > entity_data.shield[i].recharge_time) {
            entity_data.shield[i].value = Math::min_i(entity_data.shield[i].max, entity_data.shield[i].value + entity_data.shield[i].recharge);
            entity_data.shield[i].timer = 0.0f;
        }
    }
}

template<typename T, typename TargetT>
void system_homing(T &homing_entities, TargetT &lookup) {
    for(int i = 0; i < homing_entities.length; i++) {
        HomingComponent &homing = homing_entities.homing[i];
        const float radius = homing.radius;
        if(radius > 0) {
            if(!homing.has_target) {
                // find first target in radius
                for(int li = 0; li < lookup.length; li++) {
                    if(Math::distance_v(homing_entities.position[i].value, lookup.position[li].value) < radius) {
                        homing.target = lookup.entity[li];
                        homing.has_target = true;
                        break;
                    }
                }
            }

            if(homing.has_target) {
                auto handle = lookup.get_handle(homing.target);
                if(lookup.is_valid(handle)) {
                    Vector2 &s = homing_entities.position[i].value;
                    Vector2 &target = lookup.position[handle.i].value;

                    /*
                        local projectile_heading = Vector(self.collider:getLinearVelocity()):normalized()
                        local angle = math.atan2(self.target.y - self.y, self.target.x - self.x)
                        local to_target_heading = Vector(math.cos(angle), math.sin(angle)):normalized()
                        local final_heading = (projectile_heading + 0.1*to_target_heading):normalized()
                        self.collider:setLinearVelocity(self.v*final_heading.x, self.v*final_heading.y)
                    */

                    // Follow a curve
                    auto projectile_heading = homing_entities.velocity[i].value.normal();
                    float angle = Math::rads_between_v(s, target);
                    auto to_target_heading = Vector2(Math::cos_f(angle), Math::sin_f(angle)).normal();
                    auto final_heading = (projectile_heading + 0.1f * to_target_heading).normal();
                    
                    homing_entities.velocity[i].value = final_heading * homing_entities.velocity[i].value.length();

                    
                    // Straight to the point
                    // auto rotation = Math::rads_between_f(s.x, s.y, target.x, target.y);
                    // Vector2 direction_to_target = Vector2(cos(rotation), sin(rotation));
                    // projectiles.velocity[i].value = direction_to_target * bp_spd();
                } else {
                    homing.has_target = false;
                }
            }
        }
    }
}

template<typename T>
void system_on_death(T &entity_data, GameAreaController *game_ctrl) {
    for(int i = 0; i < entity_data.length; i++) {
        if(entity_data.life_time[i].marked_for_deletion) {
            if(entity_data.on_death[i].explosion) {
                game_ctrl->spawn_player_explosion_projectile(entity_data.position[i].value);
            }
        }
    }
}

#endif