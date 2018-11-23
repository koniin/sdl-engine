#ifndef COLLISION_TESTS_H
#define COLLISION_TESTS_H

#include "engine.h"
#include "renderer.h"

struct Circle {
    Vector2 position;
    uint16_t radius;
    Vector2 velocity;
};

struct Line {
    Point a;
    Point b;
};

struct Gun {
    Vector2 position;
    Point forward;
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
bool use_delta_time_speed = false;
Rectangle world_bounds;

float bullet_life = 0.0f;

std::vector<Line> lines;
Line master_line;
Point global_gun_collision;
Point global_circle_collision;
Point global_circle_collision2;
Point global_circle_collision3;
Point global_bullet_collision;

bool has_collision1 = false;
bool has_collision2 = false;
bool has_bullet_collision = false;

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

void update_gun() {
    float x_direction, y_direction;
    float rotation = gun.angle / Math::RAD_TO_DEGREE;
    x_direction = cos(rotation);
    y_direction = sin(rotation);
    gun.forward.x = (int)(gun.position.x + x_direction * 30);
    gun.forward.y = (int)(gun.position.y + y_direction * 30);
}

SpriteSheet the_sheet;
void collision_test_load() {
    Engine::set_base_data_folder("data");
    Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);
    FrameLog::enable_at(5, 5);
	Resources::sprite_load("shooter", "shooter_spritesheet");

    circles = new Circle[max_circles];
    circles[0].position = Vector2((float)gw / 2, (float)gh / 2);
    circles[0].radius = 6;
    
    circles[1].position = Vector2((float)gw / 2 - 40, (float)gh / 2);
    circles[1].radius = 4;
    circle_n = 2;

    bullets = new Circle[max_bullets];

    gun.position = Vector2(200, (float)gh / 2);
    gun.angle = 0;
    update_gun();

    world_bounds = { 0, 0, (int)gw, (int)gh };

    master_line.a = Point(100, (int)gh / 2);
    master_line.b = Point(100, (int)gh / 2 + 20);
    for(int i = 0; i < 10; ++i) {
        Line l;
        RNG::random_point_i((int)gw - 100, gh - 100, l.a.x, l.a.y);
        l.b.x = l.a.x + RNG::range_i(-20, 20);
        l.b.y = l.a.y + RNG::range_i(-20, 20);
        lines.push_back(l);
    }
    
    global_gun_collision.x = global_circle_collision.x = global_circle_collision2.x = -100;
    global_gun_collision.y = global_circle_collision.y = global_circle_collision2.y = -100;
}

bool circle_contains_point(Vector2 circle, float radius, Vector2 point) {
    float circle_left = circle.x - radius;
    float circle_right = circle.x + radius;
    float circle_bottom = circle.y + radius;
    float circle_top = circle.y - radius;
    //  Check if point is inside bounds
    if (radius > 0 && point.x >= circle_left && point.x <= circle_right && point.y >= circle_top && point.y <= circle_bottom) {
        float dx = (circle.x - point.x) * (circle.x - point.x);
        float dy = (circle.y - point.y) * (circle.y - point.y);
        return (dx + dy) <= (radius * radius);
    }
    
    return false;
}

// From Phaser
// Works well and can detect if a line is inside a circle also
bool intersect_line_circle(const Vector2 &lineP1, const Vector2 &lineP2, const Vector2 &circle_center, const float &radius, Vector2 &nearest) {
    if (circle_contains_point(circle_center, radius, lineP1)) {
        nearest.x = lineP1.x;
        nearest.y = lineP1.y;
        // furthest.x = lineP2.x;
        // furthest.y = lineP2.y;
        return true;
    }

    if (circle_contains_point(circle_center, radius, lineP2)) {
        nearest.x = lineP2.x;
        nearest.y = lineP2.y;
        // furthest.x = lineP1.x;
        // furthest.y = lineP1.y;
        return true;
    }

    float dx = lineP2.x - lineP1.x;
    float dy = lineP2.y - lineP1.y;

    float lcx = circle_center.x - lineP1.x;
    float lcy = circle_center.y - lineP1.y;

    //  project lc onto d, resulting in vector p
    float dLen2 = (dx * dx) + (dy * dy);
    float px = dx;
    float py = dy;

    if (dLen2 > 0) {
        float dp = ((lcx * dx) + (lcy * dy)) / dLen2;
        px *= dp;
        py *= dp;
    }

    nearest.x = lineP1.x + px;
    nearest.y = lineP1.y + py;
    
    //  len2 of p
    float pLen2 = (px * px) + (py * py);
    return pLen2 <= dLen2 && ((px * dx) + (py * dy)) >= 0 && circle_contains_point(circle_center, radius, nearest);
}

// It works for intersection, not much more to say
bool intersect_line_circle2(const Vector2 &a, const Vector2 &b, const Vector2 &center, const float &radius, Vector2 &n, float &depth) {
	Vector2 ap = center - a;
	Vector2 ab = b - a;
    
    float dab = Math::dot_product(ab, ab);
	
	if (dab == 0.0f) { 
        return false;
    }

    depth = Math::clamp(Math::dot_product(ap, ab) / dab, 0.0f, 1.0f);
	n = center - (a + ab * depth);
	depth = n.length2();
	if (depth > radius * radius) {
		return false;
    }
	n = n.normal();
	depth = radius - Math::sqrt_f(depth);
	return true;
}

// Imaginary line through of inifinite length
// Checks an infinite length line against a circle
// Perhaps good with rays that pierce or something?
size_t intersect_line_circle3(
    float cx, float cy, float radius,
    Vector2 point1, Vector2 point2,
    Vector2 &intersection1, Vector2 &intersection2)
{
    float dx, dy, A, B, C, det, t;

    dx = point2.x - point1.x;
    dy = point2.y - point1.y;

    A = dx * dx + dy * dy;
    B = 2 * (dx * (point1.x - cx) + dy * (point1.y - cy));
    C = (point1.x - cx) * (point1.x - cx) +
        (point1.y - cy) * (point1.y - cy) -
        radius * radius;

    det = B * B - 4 * A * C;
    if ((A <= 0.0000001f) || (det < 0))
    {
        return 0;
    }
    else if (det == 0)
    {
        // One solution.
        t = -B / (2 * A);
        intersection1 = Vector2(point1.x + t * dx, point1.y + t * dy);
        return 1;
    }
    else
    {
        // Two solutions.
        t = (float)((-B + Math::sqrt_f(det)) / (2 * A));
        intersection1 = Vector2(point1.x + t * dx, point1.y + t * dy);
        t = (float)((-B - Math::sqrt_f(det)) / (2 * A));
        intersection2 = Vector2(point1.x + t * dx, point1.y + t * dy);
        return 2;
    }
}

// Works good and finds the entry point of collision
// return values:
// 0: no collision
// 1: collision but no entry/exit point
// 2: collision and entry/exit point closest to segment_start
int intersect_line_circle4(const Vector2 &segment_start, const Vector2 &segment_end, const Vector2 &center, const float &radius, Vector2 &intersection) {
    // if (circle_contains_point(center, radius, segment_start)) {
    //     return true;
    // }

    // if (circle_contains_point(center, radius, segment_end)) {
    //     return true;
    // }
    
    /*
    Taking

    E is the starting point of the ray,
    L is the end point of the ray,
    C is the center of sphere you're testing against
    r is the radius of that sphere

    Compute:
    d = L - E ( Direction vector of ray, from start to end )
    f = E - C ( Vector from center sphere to ray start ) 
    */
    Vector2 d = segment_end - segment_start;
    Vector2 f = segment_start - center;
    float r = radius;

    float a = d.dot( d ) ;
    float b = 2*f.dot( d ) ;
    float c = f.dot( f ) - r*r ;

    float discriminant = b*b-4*a*c;
    if( discriminant < 0 ) {
        // no intersection
        return 0;
    }
   
    // ray didn't totally miss sphere,
    // so there is a solution to
    // the equation.
    discriminant = Math::sqrt_f( discriminant );

    // either solution may be on or off the ray so need to test both
    // t1 is always the smaller value, because BOTH discriminant and
    // a are nonnegative.
    float t1 = (-b - discriminant)/(2*a);
    float t2 = (-b + discriminant)/(2*a);
    
    // 3x HIT cases:
    //          -o->             --|-->  |            |  --|->
    // Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

    // 3x MISS cases:
    //       ->  o                     o ->              | -> |
    // FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

    if(t1 <= 0 && t2 >= 1) {
        // Completely inside
        // we consider this a hit, not a miss
        Engine::logn("inside");
        return 1;
    }

    if(t1 >= 0 && t1 <= 1)
    {
        // t1 is the intersection, and it's closer than t2
        // (since t1 uses -b - discriminant)
        // Impale, Poke
        Engine::logn("impale, poke");
        intersection = Vector2(segment_start.x + t1 * d.x, segment_start.y + t1 * d.y);
        return 2;
    }

    // here t1 didn't intersect so we are either started
    // inside the sphere or completely past it
    if(t2 >= 0 && t2 <= 1)
    {
        // ExitWound
        Engine::logn("exit wound");
        intersection = Vector2(segment_start.x + t1 * d.x, segment_start.y + t1 * d.y);
        return 2;
    }

    // no intn: FallShort, Past,  // CompletelyInside
    return 0;    
}

void collision_test_update() {
    if(Input::key_down(SDL_SCANCODE_A)) {
        gun.angle -= 5;
        update_gun();
    } else if(Input::key_down(SDL_SCANCODE_D)) {
        gun.angle += 5;
        update_gun();
    }
    if(Input::key_down(SDL_SCANCODE_W)) {
        float rotation = gun.angle / Math::RAD_TO_DEGREE;
        float x_direction, y_direction;
        x_direction = cos(rotation);
        y_direction = sin(rotation);
        gun.position.x += x_direction * 1;
        gun.position.y += y_direction * 1;
        update_gun();
    } 

    if(Input::key_pressed(SDLK_p)) {
        camera_shake(0.2f);
    }

    if(Input::key_pressed(SDLK_u)) {
        bullet_velocity += 2;
    }
    if(Input::key_pressed(SDLK_j)) {
        bullet_velocity -= 1;
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

    has_collision1 = false;
    has_collision2 = false;

    for(int bi = 0; bi < bullet_n; bi++) {
        const auto &bullet_pos = bullets[bi].position;
        const auto &bullet_last_pos = bullets[bi].position - bullets[bi].velocity;
        //const float &b_radius = bullets[bi].radius;
        /*
        This can be used to check the circle front, the current one only checks for circle center and last pos
        const auto &b_pos = bullets[bi].position;
        const auto &bullet_last_pos = bullets[bi].position - bullets[bi].velocity;
        Vector2 bullet_pos = b_pos + (bullets[bi].radius * bullets[bi].velocity.normal());
        */

        float distance_to_closest = (float)gw;
        Vector2 collision_point;
        bool collided = false;
        int collision_id = 0;
        bool has_collision_point = false;
        for(int ci = 0; ci < circle_n; ci++) {
            const auto &circle_pos = circles[ci].position;
            const float &circle_radius = circles[ci].radius;
        
            // if(Math::intersect_circles(circle_pos.x, circle_pos.y, circle_radius, bullet_pos.x, bullet_pos.y, b_radius)) {
            //     Engine::logn("circle intersection");
            //     continue;
            // }
            
            Vector2 nearest;
            int result = intersect_line_circle4(bullet_last_pos, bullet_pos, circle_pos, circle_radius, nearest);
            if(result == 1 || result == 2) {
                collided = true;
                float dist = Math::distance_v(bullet_last_pos, circle_pos);
                if(dist < distance_to_closest) {
                    dist = distance_to_closest;
                    has_collision_point = false;
                    if(result == 2) {
                        collision_point = nearest;
                        has_collision_point = true;
                    }
                }
                collision_id += 2;
            }

            Vector2 nearest_point;
            if(intersect_line_circle(bullet_last_pos, bullet_pos, circle_pos, circle_radius, nearest_point)) {
                // global_circle_collision2 = nearest_point.to_point();
                // Engine::logn("\t collider x: %.4f , y: %.4f", collider.x, collider.y);
                //Engine::logn("collision");
                
                collision_id += 3;
            }
        }

        if(collision_id == 2 || collision_id == 3) {
            Engine::logn("only one collision check reported hit: %d", collision_id);
        }

        if(collided) {
            if(has_collision_point) {
                has_bullet_collision = true;
                global_bullet_collision = collision_point.to_point();
            }

            bullets[bi].position.x = (float)gw * 2;
        }
    }

    for(int ci = 0; ci < circle_n; ci++) {
        const auto &circle_pos = circles[ci].position;
        const float &circle_radius = circles[ci].radius;
        
            // Vector2 collider, collider2;
        // intersections = intersect_line_circle(gun.forward.to_vector2(), gun.position, circle_pos, circle_radius, collider, collider2);
        // if(intersections) {
        //     global_circle_collision = collider.to_point();
        //     global_circle_collision2 = collider2.to_point();
        // }
        // Vector2 collider;
        // float t;
        // if(intersect_line_circle2(gun.forward.to_vector2(), gun.position, circle_pos, circle_radius, collider, t)) {
        //     global_circle_collision = (circle_pos + collider * t).to_point();
        //     //Engine::logn("t? %.0f   \t collider x: %.4f , y: %.4f", t, collider.x, collider.y);
        //     has_collision1 = true;
        // }
        
        Vector2 collider;
        float p1 = 0, p2 = 0;
        int a = intersect_line_circle4(gun.position, gun.forward.to_vector2(), circle_pos, circle_radius, collider);
        if(a == 1 || a == 2) {
            has_collision1 = true;
            global_circle_collision = collider.to_point();
            Engine::logn("p? %.0f, %.0f", p1, p2);
        }


        if(intersect_line_circle(gun.forward.to_vector2(), gun.position, circle_pos, circle_radius, collider)) {
            global_circle_collision2 = collider.to_point();
            // Engine::logn("\t collider x: %.4f , y: %.4f", collider.x, collider.y);
            //Engine::logn("collision");
            has_collision2 = true;
        }
    }

    for(auto &l : lines) {
        Vector2 collider;
        if(Math::intersect_lines_vector(gun.forward.to_vector2(), gun.position, l.a.to_vector2(), l.b.to_vector2(), collider)) {
            global_gun_collision = collider.to_point();
        }
    }

    std::string delta = use_delta_time_speed ? "yes" : "no";
    FrameLog::log("Delta movement: " + delta);
    FrameLog::log("Bullet count: " + std::to_string(bullet_n));
    FrameLog::log("Bullet velocity: " + std::to_string(bullet_velocity));
    FrameLog::log("Bullet velocity_delta: " + std::to_string(bullet_velocity_delta));
    FrameLog::log("Bullet radius: " + std::to_string(bullet_radius));
    FrameLog::log("Bullet life: " + std::to_string(bullet_life));
    std::string c1 = has_collision1 ? "yes" : "no";
    FrameLog::log("Collision method 1: " + c1);
    std::string c2 = has_collision2 ? "yes" : "no";
    FrameLog::log("Collision method 2: " + c2);
    //FrameLog::log("Intersections: " + std::to_string(intersections));
}

void collision_test_render() {
    for(int i = 0; i < circle_n; i++) {
        const auto &pos = circles[i].position;
        draw_g_circe_RGBA((int)pos.x, (int)pos.y, circles[i].radius, 123, 255, 123, 255);
    }
    for(int i = 0; i < bullet_n; i++) {
        const auto &b_pos = bullets[i].position;
        Vector2 pos = b_pos + (bullets[i].radius * bullets[i].velocity.normal());
        const auto &last_pos = bullets[i].position - bullets[i].velocity;
        
        draw_g_line_RGBA((int)pos.x, (int)pos.y, (int)last_pos.x, (int)last_pos.y, 0, 255, 0, 255);
        draw_g_circe_RGBA((int)b_pos.x, (int)b_pos.y, bullets[i].radius, 0, 255, 0, 255);
    }
    for(auto &l : lines) {
        draw_g_line_RGBA(l.a.x, l.a.y, l.b.x, l.b.y, 128, 0, 255, 255);
    }
    draw_g_line_RGBA((int)gun.position.x, (int)gun.position.y, gun.forward.x, gun.forward.y, 0, 0, 255, 255);
    draw_g_circe_RGBA((int)gun.position.x, (int)gun.position.y, 2, 255, 255, 0, 255);
    draw_g_circe_RGBA(global_gun_collision.x, global_gun_collision.y, 2, 255, 0, 0, 255);
    if(has_collision1) {
        draw_g_circe_RGBA(global_circle_collision.x, global_circle_collision.y, 2, 255, 0, 0, 255);
        draw_g_circe_RGBA(global_circle_collision3.x, global_circle_collision3.y, 2, 255, 0, 0, 255);
    }
    if(has_collision2) {
        draw_g_circe_RGBA(global_circle_collision2.x, global_circle_collision2.y, 2, 255, 150, 150, 255);
    }
    if(has_bullet_collision) {
        draw_g_circe_RGBA(global_bullet_collision.x, global_bullet_collision.y, 2, 255, 150, 150, 255);
    }
}

void render_after_render_target() {
    // draw_sprite_centered(Resources::sprite_get("shooter"), -10, -10);
}

#endif