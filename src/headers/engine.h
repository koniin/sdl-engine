#ifndef ENGINE_H
#define ENGINE_H

#include "SDL.h"
#include <algorithm>
#include <vector>
#include <random>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>

#ifdef _DEBUG
#define ASSERT_WITH_MSG(cond, msg) do \
{ if (!(cond)) { std::ostringstream str; str << msg; std::cerr << str.str(); std::abort(); } \
} while(0)
#else 
#define ASSERT_WITH_MSG(cond, msg) ;
#endif

// timer
typedef struct {
    Uint64 now;
    Uint64 last;
    double dt;
    double fixed_dt;
    double accumulator;
} gameTimer;


// Find functions here if you need them
// https://github.com/nicolausYes/easing-functions/blob/master/src/easing.cpp

inline float easing_linear(float t) {
    return t;
}

inline float easing_InSine(float t) {
	return sinf( 1.5707963f * t );
}

inline float easing_OutSine(float t) {
	return 1 + sinf( 1.5707963f * (--t) );
}

inline float easing_sine_in_out(float t) {
	return 0.5f * (1 + sinf( 3.1415926f * (t - 0.5f) ) );
}

typedef float (*easing_t)(float);

namespace Engine {
	static int32_t current_fps = 0;
	static bool _is_running = true;
	inline void exit() {
		_is_running = false;
	}
	inline bool is_running() {
		return _is_running;
	}
	bool is_paused();
	void toggle_logging();
	void log(const char* fmt, ...);
	void logn(const char* fmt, ...);

	void set_base_data_folder(const std::string &name);
	inline std::string get_base_data_folder();

	void pause(float time);

	void update();
}

namespace Text {
	inline std::string format(const std::string format, ...) {
        va_list args;
        va_start (args, format);
        size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
        va_end (args);
        std::vector<char> vec(len + 1);
        va_start (args, format);
        std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
        va_end (args);
        return &vec[0];
    }
};

struct Vector2;
struct Point {
	int x;
	int y;
	Point() { x = 0; y = 0; };
	Point(int xPos, int yPos) {
		x = xPos;
		y = yPos;
	}
	
	template<class T>
	inline Point operator=(const T &v) const {
		return Point((int)v.x, (int)v.y);
	}
	inline Point operator-(Point const& point) const {
		return Point(x - point.x, y - point.y);
	}
	inline Point Point::operator*(int const &num) const {
		return Point(x * num, y * num);
	}
	
	Vector2 to_vector2() const;
};
inline Point operator+( Point const& lhs, Point const& rhs ) {
	return Point(lhs.x + rhs.x, lhs.y + rhs.y);
}
inline bool operator==( Point const& lhs, Point const& rhs ) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}
inline Point operator+=(Point const& lhs, int const& rhs){
	return Point(lhs.x + rhs, lhs.y + rhs);
}
inline Point operator-=(Point const& lhs, int const& rhs){
	return Point(lhs.x - rhs, lhs.y - rhs);
}
inline const Point operator*(int lhs, Point const &rhs) {
	Point result;
	result.x=rhs.x * lhs;
	result.y=rhs.y * lhs;
	return result;
}

const float vector_mag(Vector2 const &rhs);
struct Vector2 {
	float x;
	float y;
	Vector2() { x = 0; y = 0; };
	explicit Vector2(float xPos, float yPos) {
		x = xPos;
		y = yPos;
	}
	
	static const Vector2 Zero;
	static const Vector2 One;
	
	inline Vector2& Vector2::operator+=(const Vector2& vector) {
		x += vector.x;
		y += vector.y;
		return *this;
	}
	inline Vector2& Vector2::operator-=(const Vector2& vector) {
		x -= vector.x;
		y -= vector.y;
		return *this;
	}
	inline Vector2& Vector2::operator*=(const Vector2& vector) {
		x *= vector.x;
		y *= vector.y;
		return *this;
	}
	inline Vector2& Vector2::operator/=(const Vector2& vector) {
		x /= vector.x;
		y /= vector.y;
		return *this;
	}
	inline Vector2 Vector2::operator-(const Vector2& vector) const {
			return Vector2(x - vector.x, y - vector.y);
	}
	inline Vector2 Vector2::operator*(const Vector2& vector) const {
			return Vector2(x * vector.x, y * vector.y);
	}
	inline Vector2 Vector2::operator-(float const &num) const {
		return Vector2(x - num, y - num);
	}
	inline Vector2 Vector2::operator+(float const &num) const {
		return Vector2(x + num, y + num);
	}
	inline Vector2 Vector2::operator*(float const &num) const {
		return Vector2(x * num, y * num);
	}
	inline Vector2 Vector2::operator/(float const &num) const {
		return Vector2(x / num, y / num);
	}
	inline Vector2& Vector2::operator+=(const float &num) {
		x += num;
		y += num;
		return *this;
	}
	inline Vector2& Vector2::operator-=(const float &num) {
		x -= num;
		y -= num;
		return *this;
	}
	inline Vector2& Vector2::operator*=(const float &num) {
		x *= num;
		y *= num;
		return *this;
	}
	inline Vector2& Vector2::operator/=(const float &num) {
		x /= num;
		y /= num;
		return *this;
	}
	inline bool Vector2::operator==(const Vector2& vector) const {
		return x == vector.x && y == vector.y;
	}
	inline bool Vector2::operator!=(const Vector2& vector) const {
		return x != vector.x || y != vector.y;
	}

	Point to_point() const;
	Vector2 normal() const;
	float length2() const;
};

inline Vector2 operator+( Vector2 const& lhs, Vector2 const& rhs ) {
	return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}
inline const Vector2 operator*(float lhs, Vector2 const &rhs) {
	Vector2 result;
	result.x=rhs.x * lhs;
	result.y=rhs.y * lhs;
	return result;
}

//Returns dot product
inline const float vector_dot(Vector2 const &lhs, Vector2 const &rhs) {
	return lhs.x*rhs.x+lhs.y*rhs.y; 
}
//Returns length squared
inline const float vector_lsq(Vector2 const &rhs) {
	return vector_dot(rhs, rhs);
}
//Returns magnitude (length)
inline const float vector_mag(Vector2 const &rhs) {
	//return sqrtf(dot(rhs, rhs));
	return sqrt(vector_dot(rhs, rhs));
}
//Returns normalized Vector2
inline Vector2 vector_norm(Vector2 const &lhs){
	return (1.f /(vector_mag(lhs))) * lhs;
}

struct Rectangle {
	int x;
	int y;
	int w;
	int h;

	Rectangle() {
		x = y = w = h = 0;
	}

	Rectangle(int xx, int yy, int ww, int hh) {
		x = xx;
		y = yy;
		w = ww;
		h = hh;
	}

	bool contains(int xi, int yi) {
		return ((((x <= xi) && (xi < (x + w))) && (y <= yi)) && (yi < (y + h)));
	}

	bool contains(Point &p) {
		return contains(p.x, p.y);
	}

	int left() {
        return x;
    }

	int right() {
        return x + w;
    }

	int top() {
        return y;
    }

	int bottom() {
        return y + h;
    }

	bool intersects(Rectangle &r2) {
        return !(r2.left() > right()
                 || r2.right() < left()
                 || r2.top() > bottom()
                 || r2.bottom() < top()
                );
    }
};

namespace Time {
	extern float deltaTime;
}

namespace Input {
    extern int mousex;
	extern int mousey;

    void init();
    void update_states();
    void map(const SDL_Event *event);
    bool key_down(const SDL_Scancode &scanCode);
    bool key_released(const SDL_Keycode &keyCode);
    bool key_pressed(const SDL_Keycode &keyCode);
}

struct Scene {
	virtual void init() = 0;
  	virtual void cleanup() = 0;
	virtual void update(float dt) = 0;
	virtual void render() = 0;
};

namespace Scenes {
	void set_scene(Scene* scene);
	Scene* get_current();
	void free();
	void update(float dt);
	void render();
}

namespace Math {
	static const float Pi = 3.14159265358979323846f;
	static const float RAD_TO_DEGREE = 180.0f / (float)M_PI;
	
	inline float clamp(float x, float a, float b) {
    	x = std::fmax(x, a);
    	x = std::fmin(x, b);
    	return x;
	}

	inline int max(int a, int b) {
		return std::max(a, b);
	}

	inline float max_f(float a, float b) {
		return std::fmax(a, b);
	}

	inline int min(int a, int b) {
		return std::min(a, b);
	}

	inline float min_f(float a, float b) {
		return std::fmin(a, b);
	}

	inline float ceiling(float f) {
		return std::ceilf(f);
	}

	inline int abs(int i) {
		return std::abs(i);
	}

	inline float abs_f(float f) {
		return std::abs(f);
	}
	
	inline float round(float f) {
		return std::round(f);
	}

	inline float round_bankers(float f) {
		if(f == 0.5f)
			return 0.0f;
		return std::round(f);
	}

	inline float lerp(float from, float to, float t) {
    	return from + (to - from) * t;
	}

	inline float length_vector_f(float x, float y) {
		return sqrt(x*x + y*y);
	}

	inline float interpolate(float from, float to, float amount, easing_t easing) {
    	return from + (to - from) * (easing(amount));
	}

	inline void interpolate_point(Point *target, const Point &from, const Point &to, const float amount, easing_t easing) {
		target->x = (int)(from.x + (to.x - from.x) * (easing(amount)));
		target->y = (int)(from.y + (to.y - from.y) * (easing(amount)));
	}

	inline void interpolate_vector(Vector2 *target, const Vector2 &from, const Vector2 &to, const float amount, easing_t easing) {
		target->x = from.x + (to.x - from.x) * (easing(amount));
		target->y = from.y + (to.y - from.y) * (easing(amount));
	}

	inline float distance_f(const float &x1, const float &y1, const float &x2, const float &y2) {
		auto dx = x1 - x2;
    	auto dy = y1 - y2;
		return sqrt(dx * dx + dy * dy);
	}

	inline float distance_v(const Vector2 &a, const Vector2 &b) {
		auto dx = a.x - b.x;
    	auto dy = a.y - b.y;
		return sqrt(dx * dx + dy * dy);
	}

	inline float rads_between_f(const float &x1, const float &y1, const float &x2, const float &y2) {
    	return atan2(y2 - y1, x2 - x1);
	}

	inline float rads_between_v(const Vector2 &a, const Vector2 &b) {
    	return atan2(b.y - a.y, b.x - a.x);
	}

	inline float rads_to_degrees(float rads) {
		return rads * RAD_TO_DEGREE;
	}

	inline bool intersect_circles(float c1X, float c1Y, float c1Radius, float c2X, float c2Y, float c2Radius) {
		float distanceX = c2X - c1X;
		float distanceY = c2Y - c1Y;     
		float magnitudeSquared = distanceX * distanceX + distanceY * distanceY;
		return magnitudeSquared < (c1Radius + c2Radius) * (c1Radius + c2Radius);
	}

	struct AABB {
		int left;
		int right;
		int bottom;
		int top;
	};

	struct AABB_f {
		float left;
		float right;
		float bottom;
		float top;
	};

	inline bool intersect_AABB(const Rectangle &a, const Rectangle &b) {
		AABB aa = { a.x, a.x + a.w, a.y, a.y + a.h };
		AABB bb = { b.x, b.x + b.w, b.y, b.y + b.h };
  		return (aa.left <= bb.right && aa.right >= bb.left) 
			&& (aa.bottom <= bb.top && aa.top >= bb.bottom);
	}

	inline bool intersect_circle_AABB(const float &cx, const float &cy, const float &radius, const Rectangle &rect) {
		AABB_f aa = { (float)rect.x, (float)rect.x + rect.w, (float)rect.y, rect.y + (float)rect.h };
		float delta_x = cx - Math::max_f(aa.left, Math::min_f(cx, aa.right));
		float delta_y = cy - Math::max_f(aa.bottom, Math::min_f(cy, aa.top));
		return (delta_x * delta_x + delta_y * delta_y) < (radius * radius);
	}

	inline bool intersect_lines_vector(const Vector2 &lineA1, const Vector2 &lineA2, const Vector2 &lineB1, const Vector2 &lineB2, Vector2 &collision_point) {
		float denom = ((lineB2.y - lineB1.y) * (lineA2.x - lineA1.x)) -
			((lineB2.x - lineB1.x) * (lineA2.y - lineA1.y));
		
		if (denom == 0) {
			return false;
		}
		
		float ua = (((lineB2.x - lineB1.x) * (lineA1.y - lineB1.y)) -
			((lineB2.y - lineB1.y) * (lineA1.x - lineB1.x))) / denom;
		/* The following 3 lines are only necessary if we are checking line
			segments instead of infinite-length lines */
		float ub = (((lineA2.x - lineA1.x) * (lineA1.y - lineB1.y)) -
			((lineA2.y - lineA1.y) * (lineA1.x - lineB1.x))) / denom;
		if ((ua < 0) || (ua > 1) || (ub < 0) || (ub > 1)) {
			return false;
		}

		collision_point = lineA1 + ua * (lineA2 - lineA1);
		return true;
	}
	// 	inline bool intersect_AABB(Rectangle &a, Rectangle &b) {
	//   		return (a.minX <= b.maxX && a.maxX >= b.minX) 
	// 			&& (a.minY <= b.maxY && a.maxY >= b.minY);
	// 	}

	// 	public static intersect(r1:Rectangle, r2:Rectangle):boolean {
	//         return !(r2.left > r1.right || 
	//            r2.right < r1.left || 
	//            r2.top > r1.bottom ||
	//            r2.bottom < r1.top);
	//     }

	//     public static intersectXY(x1:number, y1:number, width1:number, height1:number, x2:number, y2:number, width2:number, height2:number):boolean {
	//         return !(x2 > x1 + width1 || 
	//            x2 + width2 < x1 || 
	//            y2 > y1 + height1 ||
	//            y2 + height2 < y1);
	// }

	// call repeatedly to move to target 
	// returns true when at target
	// speed_per_tick => e.g. 4 pixels per call
	inline bool move_to(float &x, float &y, float targetX, float targetY, float speed) {
		float delta_x = targetX - x;
		float delta_y = targetY - y;
		float goal_dist = sqrt( (delta_x * delta_x) + (delta_y * delta_y) );
		if (goal_dist > speed) {
			float ratio = speed / goal_dist;
			float x_move = ratio * delta_x;  
			float y_move = ratio * delta_y;
			x = x_move + x;
			y = y_move + y;
			return false;
		}
		
		// destination reached
		return true;
	}
}

namespace RNG {
	static std::random_device RNG_seed;
	static std::mt19937 RNG_generator(RNG_seed());

    inline int next_i(int max) {
        std::uniform_int_distribution<int> range(0, max);
        return range(RNG_generator);
    }

    inline int range_i(int min, int max) {
        std::uniform_int_distribution<int> range(min, max);
        return range(RNG_generator);
    }

    inline float range_f(float min, float max) {
        std::uniform_real_distribution<float> range(min, max);
        return range(RNG_generator);
    }

	inline void random_point_i(int xMax, int yMax, int &xOut, int &yOut) {
		std::uniform_int_distribution<int> xgen(0, xMax);
		std::uniform_int_distribution<int> ygen(0, yMax);
		xOut = xgen(RNG_generator);
		yOut = ygen(RNG_generator);
	}

	inline Vector2 vector2(const float &x_min, const float &x_max, const float &y_min, const float &y_max) {
		std::uniform_real_distribution<float> xgen(x_min, x_max);
		std::uniform_real_distribution<float> ygen(y_min, y_max);
		return Vector2(xgen(RNG_generator), ygen(RNG_generator));
	}
}

namespace Localization {
    struct Text {
        Text(const char *file);
        char *getText(const std::string s);
        std::map<std::string, char *> texts;
    };

    void load_texts(const char *file);
    char *text_lookup(const std::string s);
}

namespace Serialization {	
	void write_string(std::ostream &out, const std::string &s);
	void read_string(std::istream &in, std::string &s);
}

namespace Noise {
	// All noise functions from
	// https://github.com/Auburns/FastNoise
	// Lowest amount to highest amount interpolation
	enum Interp { Linear, Hermite, Quintic };
	
	void init(int seed = 1337);
	void set_seed(int seed);	
	float perlin(float x, float y);
	float simplex(float x, float y);
}

struct TileMap {
	unsigned tile_size;
	unsigned columns;
	unsigned rows;
	unsigned layers;
	unsigned *tiles;
};

namespace Tiling {
	unsigned tilemap_index(const TileMap &tile_map, const unsigned layer, const unsigned x, const unsigned y);
	void tilemap_load(const std::string map_name, TileMap &tile_map);
	void tilemap_make(TileMap &tile_map, unsigned layers, unsigned columns, unsigned rows, unsigned tile_size, unsigned default_tile = 0);
}

/*
struct GameEvent {
    int id;
    std::shared_ptr<void> data;
};

struct EventQueue {
    int listener_count;
    void (*listener_list[255])(int, std::shared_ptr<void>);
    int queue_count;
    GameEvent queue[255];
};

void eventqueue_Init(EventQueue *e);
void eventqueue_RemoveListener(EventQueue *e, void (*fp)(int, std::shared_ptr<void>));
void eventqueue_AddListener(EventQueue *e, void (*fp)(int, std::shared_ptr<void>));
void eventqueue_TriggerEvent(EventQueue *e, const int eventId, std::shared_ptr<void> data);
void eventqueue_QueueEvent(EventQueue *e, const int eventId, std::shared_ptr<void> data);
void eventqueue_PumpQueuedEvents(EventQueue *e);

namespace GlobalEvents {
    void init();
    void update();
    EventQueue *getGlobalEventQueue();
}
*/

/*
namespace Animation {
	struct Animation {
		int frame = 0;
		float timer = 0;
		float fps = 3;
		float duration = 1;
		bool loop = false;
		std::string sprite_sheet;
		std::vector<SDL_Rect> frames;
	};

	struct AnimationComponent {
		std::map<std::string, std::shared_ptr<Animation>>::iterator current_animation;
		std::map<std::string, std::shared_ptr<Animation>> animatons;
	};

	std::shared_ptr<AnimationComponent> make();
	std::shared_ptr<AnimationComponent> make(std::string name, std::string sprite_sheet, std::vector<SDL_Rect> frames, float fps, bool loop = false);
	void set_current(std::shared_ptr<AnimationComponent> ac, std::string animation);
	void add(std::shared_ptr<AnimationComponent> ac, std::string name, std::string sprite_sheet, std::vector<SDL_Rect> frames, float fps, bool loop = false);
	void update(std::shared_ptr<AnimationComponent> ac, float dt);
	void get_sprite_sheet(const std::string name, const std::shared_ptr<AnimationComponent> ac, std::string &sprite_sheet);
	void get_frame_size(const std::shared_ptr<AnimationComponent> ac, int &w, int &h);
}
typedef std::shared_ptr<Animation::AnimationComponent> AnimationComponent_ptr;

namespace Easing {	
	struct TargetData {
		int *target = NULL;
		int start = 0;
		int end = 0;
	};
	
	struct TargetData_f {
		float *target = NULL;
		float start = 0;
		float end = 0;
	};
	
	struct Easing_I {
		TargetData ta;
		TargetData tb;
		float timer = 0;
		float duration = 0;
		easing_t ease_func = NULL;
	};
	
	struct Easing_F {
		float *target = NULL;
		float start = 0;
		float end = 0;
		float timer = 0;
		float duration = 0;
		easing_t ease_func = NULL;
	};

	struct Easing_Point {
		Engine::Point *target = NULL;
		Engine::Point start = Engine::Point(0, 0);
		Engine::Point end = Engine::Point(0, 0);
		float timer = 0;
		float duration = 0;
		easing_t ease_func = NULL;
	};

	struct Easing_Vec2 {
		Engine::Vector2 *target = NULL;
		Engine::Vector2 start = Engine::Vector2(0, 0);
		Engine::Vector2 end = Engine::Vector2(0, 0);
		float timer = 0;
		float duration = 0;
		easing_t ease_func = NULL;
	};

	void init(int buffer_size);
	void start(int id, int *target, int end, float duration, easing_t ease_func);
	bool completed(int id, float dt);
	
	bool ease_to(float *target, float start, float end, float duration, easing_t ease_func, float *timer, float dt);
	void ping_pong(float *target, float start, float end, float duration, easing_t ease_func, float *timer, float dt);
	std::shared_ptr<Easing_F> make_float(float *target, float start, float end, float duration, easing_t ease_func);
	bool update(std::shared_ptr<Easing_F> easing, float dt);
	std::shared_ptr<Easing_Point> make_point(Engine::Point *target, Engine::Point end, float duration, easing_t ease_func);
	bool update(std::shared_ptr<Easing_Point> easing, float dt);
	std::shared_ptr<Easing_Vec2> make_vector2(Engine::Vector2 *target, Engine::Vector2 start, Engine::Vector2 end, float duration, easing_t ease_func);
	bool update(std::shared_ptr<Easing_Vec2> easing, float dt);
}

namespace Actions {	
	struct Action {
		float duration;
		float timer = 0;
		bool done = false;
		std::function<void(float dt, std::shared_ptr<Action> action)> update;
	};
	
	typedef std::shared_ptr<Actions::Action> Action_ptr;
	
	inline bool update(const float dt, const Action_ptr action) {
		action->update(dt, action);
		return action->done;
	}

	// You can not reset a parallel action!
	inline void reset(const Action_ptr action) {
		action->done = false;
		action->timer = 0.0f;
	}

	Action_ptr make_ease(float *target, float start, float end, float duration, easing_t ease_func);
	Action_ptr make_ease_vector(Engine::Vector2 *target, Engine::Vector2 start, Engine::Vector2 end, float duration, easing_t ease_func);
	Action_ptr make_func(std::function<void(void)> func);
	Action_ptr make_delay(float delay);
	Action_ptr make_parallel(Action_ptr a, Action_ptr b);
}
*/
#endif