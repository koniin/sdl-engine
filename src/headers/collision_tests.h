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

void collision_test_load() {
    Engine::set_base_data_folder("data");
    Font *font = Resources::font_load("normal", "pixeltype.ttf", 15);
	set_default_font(font);

    circles = new Circle[max_circles];
    circles[0].position = Vector2((float)gw / 2, (float)gh / 2);
    circles[0].radius = 6;
    circle_n = 1;
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

bool Contains(Vector2 circle, float radius, Vector2 point) {
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

bool intersect_line_circle(const Vector2 &lineP1, const Vector2 &lineP2, const Vector2 &circle_center, const float &radius, Vector2 &nearest) {
//   function (line, circle, nearest)
// {
    if (Contains(circle_center, radius, lineP1)) {
        nearest.x = lineP1.x;
        nearest.y = lineP1.y;
        Engine::logn("early exit 2");
        return true;
    }

    if (Contains(circle_center, radius, lineP1)) {
        nearest.x = lineP2.x;
        nearest.y = lineP2.y;
        Engine::logn("early exit 1");
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
    
    return (
        pLen2 <= dLen2 &&
        ((px * dx) + (py * dy)) >= 0 &&
        Contains(circle_center, radius, nearest)
    );
// };
}

bool intersect_line_circle2(const Vector2 &a, const Vector2 &b, const Vector2 &center, const float &radius, Vector2 &n, float &depth) {
	Vector2 ap = center - a;
	Vector2 ab = b - a;
    //float dab = ab.dot(ab);
    float dab = vector_dot(ab, ab);
	
	if (dab == 0.0f) { 
        return false;
    }

    depth = Math::clamp(vector_dot(ap, ab) / dab, 0.0f, 1.0f);
	n = center - (a + ab * depth);
	depth = n.length2();
	if (depth > radius * radius) {
		return false;
    }
	n = n.normal();
	depth = radius - sqrtf(depth);
	return true;
}

void collision_test_update() {
    FrameLog::reset();
    
    if(Input::key_down(SDL_SCANCODE_A)) {
        gun.angle -= 5;
        update_gun();
    } else if(Input::key_down(SDL_SCANCODE_D)) {
        gun.angle += 5;
        update_gun();
    }
    if(Input::key_pressed(SDLK_w)) {
        float rotation = gun.angle / Math::RAD_TO_DEGREE;
        float x_direction, y_direction;
        x_direction = cos(rotation);
        y_direction = sin(rotation);
        gun.position.x += x_direction * 1;
        gun.position.y += y_direction * 1;
        update_gun();
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

    for(int ci = 0; ci < circle_n; ci++) {
        const auto &circle_pos = circles[ci].position;
        const float &circle_radius = circles[ci].radius;
        for(int bi = 0; bi < bullet_n; bi++) {
            const auto &bullet_pos = bullets[bi].position;
            const float &b_radius = bullets[bi].radius;
            if(Math::intersect_circles(circle_pos.x, circle_pos.y, circle_radius, bullet_pos.x, bullet_pos.y, b_radius)) {
                Engine::logn("circle intersects");
            }
        }
        // Vector2 collider, collider2;
        // intersections = intersect_line_circle(gun.forward.to_vector2(), gun.position, circle_pos, circle_radius, collider, collider2);
        // if(intersections) {
        //     global_circle_collision = collider.to_point();
        //     global_circle_collision2 = collider2.to_point();
        // }
        Vector2 collider;
        // float t;
        // if(intersect_line_circle2(gun.forward.to_vector2(), gun.position, circle_pos, circle_radius, collider, t)) {
        //     global_circle_collision = collider.to_point();
        //     Engine::logn("t? %.0f   \t collider x: %.4f , y: %.4f", t, collider.x, collider.y);
        // }
        if(intersect_line_circle(gun.forward.to_vector2(), gun.position, circle_pos, circle_radius, collider)) {
            global_circle_collision = collider.to_point();
            // Engine::logn("\t collider x: %.4f , y: %.4f", collider.x, collider.y);
            Engine::logn("collision");
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
    //FrameLog::log("Intersections: " + std::to_string(intersections));
}

void collision_test_render() {
    for(int i = 0; i < circle_n; i++) {
        const auto &pos = circles[i].position;
        draw_g_circe_RGBA((uint16_t)pos.x, (uint16_t)pos.y, circles[i].radius, 0, 255, 0, 255);
    }
    for(int i = 0; i < bullet_n; i++) {
        const auto &pos = bullets[i].position;
        draw_g_circe_RGBA((uint16_t)pos.x, (uint16_t)pos.y, bullets[i].radius, 0, 255, 0, 255);
    }
    for(auto &l : lines) {
        SDL_SetRenderDrawColor(renderer.renderer, 128, 0, 255, 255);
        SDL_RenderDrawLine(renderer.renderer, l.a.x, l.a.y, l.b.x, l.b.y);
    }
    SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 255, 255);
    SDL_RenderDrawLine(renderer.renderer, (int)gun.position.x, (int)gun.position.y, gun.forward.x, gun.forward.y);
    draw_g_circe_RGBA((uint16_t)gun.position.x, (uint16_t)gun.position.y, 2, 255, 255, 0, 255);
    draw_g_circe_RGBA((uint16_t)global_gun_collision.x, (uint16_t)global_gun_collision.y, 4, 255, 0, 0, 255);
    draw_g_circe_RGBA((uint16_t)global_circle_collision.x, (uint16_t)global_circle_collision.y, 4, 255, 0, 0, 255);
    draw_g_circe_RGBA((uint16_t)global_circle_collision2.x, (uint16_t)global_circle_collision2.y, 4, 255, 0, 0, 255);

    FrameLog::render(5, 5);
}

#endif