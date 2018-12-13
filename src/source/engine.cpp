#include "engine.h"
#include <unordered_set>
#include <fstream>

#include "sound.h"

namespace FrameLog {
	void clear();
}

namespace Engine {
	int32_t current_fps = 0;

	void init() {
		Input::init();
		Sound::init();
	}

	static bool logging_enabled = true;
	void toggle_logging() {
		logging_enabled = !logging_enabled;
	}
	
	void logn(const char* fmt, ...) {
		if(!logging_enabled) {
			return;
		}
		
		va_list args;
		va_start(args, fmt);
		printf("\n");
		vprintf(fmt, args);
		va_end(args);
	}

	void log(const char* fmt, ...) {
		if(!logging_enabled) {
			return;
		}

		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
	
    static std::string base_data_folder = "";
	void set_base_data_folder(const std::string &name) {
		base_data_folder = name + "/";
	}

	std::string get_base_data_folder() {
		return base_data_folder;
	}

	static float pause_timer = 0.0f;
	bool is_paused() {
		return pause_timer > 0.0f;
	}

	void pause(float time) {
		pause_timer = time;
	}

	void update() {
		if(FrameLog::is_enabled()) {
			FrameLog::clear();
		}
		if(pause_timer > 0.0f) {
			pause_timer -= Time::delta_time_fixed;
		}
		Sound::play_all();
	}

	void cleanup() {
		Sound::quit();
	}
}

namespace FrameLog {
	const int max_messages = 20;
    static std::vector<std::string> messages;
	static Point pos;
	static bool enabled = false;

	const bool &is_enabled() {
		return enabled;
	}

    void log(const std::string &message) {
		if(!enabled) {
			return;
		}

		if(messages.size() == max_messages) {
            return;
        }
        messages.push_back(message);
    }

	void enable_at(const int x, const int y) {
        pos.x = x;
		pos.y = y;
		enabled = true;
    }

	const std::vector<std::string> &get_messages() {
		return messages;
	}

	const Point &get_position() {
		return pos;
	}

	void clear() {
		messages.clear();
	}
}

Vector2 Point::to_vector2() const {
	return Vector2((float)x, (float)y);
}

const Vector2 Vector2::Zero = Vector2(0, 0);
const Vector2 Vector2::One = Vector2(1, 1);

Point Vector2::to_point() const {
	return Point((int)x, (int)y);
}

Vector2 Vector2::normal() const {
	float mag = Math::magnitude(*this);
	if(mag == 0.0f) {
		mag = 1.0f;
	}
	return (1.f / mag) * *this;
}

float Vector2::length() const {
	return Math::sqrt_f(length2());
}

float Vector2::length2() const {
	return x*x + y*y;
}

float Vector2::dot(const Vector2 &v) const {
	return Math::dot_product(v, *this);
}

namespace Time {
	float delta_time = 0.0f;
	float delta_time_fixed = 0.0f;
	float delta_time_raw = 0.0f;
}

namespace Input {
    // class KeyboardListener {
    //     public:
    //         void OnKeyEvent(const KeyEvent& evt) {}
    // };

    // class Input {
    //     public:
    //     void AddListener(KeyboardListener* pListener) {
    //         /* add pointer to a list */
    //     }
    // };

	int mousex = 0;
	int mousey = 0;
	bool mouse_left_down = false;
    SDL_GameController *controller;
    const Uint8* current_keyboard_state;
    // Uint8* current_gamepad_button_state;
    // Uint8* previous_gamepad_button_state;
    // Sint16* gamepad_axis_state;
    // Uint8* current_mouse_state;
    // Uint8* previous_mouse_state;

    std::unordered_set<int> keysDown;
	std::unordered_set<int> keysDownNow;
	std::unordered_set<int> keysUpNow;
    // std::unordered_set<int> keysUp;
	// std::unordered_set<int> keysDown_previous;

    void init() {
        controller = NULL;
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                controller = SDL_GameControllerOpen(i);
                if (controller) break;
            }
        }
    }
 
    void update_states() {
        current_keyboard_state = SDL_GetKeyboardState(NULL);
        keysDownNow.clear();
		keysUpNow.clear();
		SDL_GetMouseState(&mousex, &mousey);
		mouse_left_down = false;
    }

    void map(const SDL_Event *event) {
        if (event->type == SDL_KEYDOWN) {
		    keysDown.insert(event->key.keysym.sym);
			keysDownNow.insert(event->key.keysym.sym);
		}
        if (event->type == SDL_KEYUP) {
			keysDown.erase(event->key.keysym.sym);
		    keysUpNow.insert(event->key.keysym.sym);
		}
		if(event->type == SDL_MOUSEBUTTONDOWN) {
			if(event->button.button == SDL_BUTTON_LEFT ) {
				mouse_left_down = true;
			}
		}
    }

    bool key_down(const SDL_Scancode &scanCode) {
        return current_keyboard_state[scanCode];
    }

	bool key_down_k(const SDL_Keycode &keyCode) {
        return keysDown.count(keyCode) > 0;
    }

    bool key_released(const SDL_Keycode &keyCode) {
        return keysUpNow.count(keyCode) > 0;
    }

    bool key_pressed(const SDL_Keycode &keyCode) {
        return keysDownNow.count(keyCode) > 0;
    }
}

namespace Scenes {
	static Scene* currentScene;

	void set_scene(Scene* scene) {
		if(currentScene != nullptr)
			currentScene->cleanup();
		delete currentScene;
		
		scene->init();
		currentScene = scene;
	}

	Scene* get_current() {
		ASSERT_WITH_MSG(currentScene != nullptr, "current scene is null");
		return currentScene;
	}

	void free() {
		if(currentScene != nullptr) {
			currentScene->cleanup();
			delete currentScene;
		}
	}

	void update(float dt) {
		ASSERT_WITH_MSG(currentScene != nullptr, "current scene is null");
		currentScene->update(dt);
	}

	void render() {
		ASSERT_WITH_MSG(currentScene != nullptr, "current scene is null");
		currentScene->render();
	}
}

namespace Localization {
	static Text *localization_text;

	Text::Text(const char *file) {
		texts.insert(std::make_pair(std::string("test"), (char*)"test result"));
		texts.insert(std::make_pair(std::string("combat_nodamage"), (char*)"BLOCK!"));
		texts.insert(std::make_pair(std::string("space_to_continue"), (char*)"Press [space] to continue"));
		texts.insert(std::make_pair(std::string("maze_death_headline"), (char*)"EMBRACE THE END"));
		texts.insert(std::make_pair(std::string("maze_death_subline"), (char*)"The maze giveth, the maze taketh away"));
		
		Engine::log("\nLocalization: actually not loading anything from file");
	}

	char *Text::getText(const std::string s) {
		if(texts.count(s) <= 0)
			return NULL;
		
		return texts[s];
	}

	void load_text(const char *file) {
		localization_text = new Text(file);
	}

	char *text_lookup(const std::string s) {
		return localization_text->getText(s);
	}
}

namespace Serialization {	
	void write_string(std::ostream &out, const std::string &s) {
    	int size = s.size();
    	out.write(reinterpret_cast<char *>(&size), sizeof(int));
    	out.write(s.c_str(), size);
	}

	void read_string(std::istream &in, std::string &s) {
		int size;
		in.read(reinterpret_cast<char *>(&size), sizeof(int));
		char *buf = new char[size];
		in.read(buf, size);
		s.append(buf, size);
		delete buf;
	}
}

namespace Noise {
	static int FastFloor(float f) { return (f >= 0 ? (int)f : (int)f - 1); }
	static float Lerp(float a, float b, float t) { return a + t * (b - a); }
	static float InterpHermiteFunc(float t) { return t*t*(3 - 2 * t); }
	static float InterpQuinticFunc(float t) { return t*t*t*(t*(t * 6 - 15) + 10); }

	static int m_seed = 1337;
	static unsigned char m_perm[512];
	static unsigned char m_perm12[512];
	static float m_frequency = float(0.01);
	static Interp m_interp = Quintic;
	
	const float GRAD_X[] = {
		1, -1, 1, -1,
		1, -1, 1, -1,
		0, 0, 0, 0
	};
	const float GRAD_Y[] = {
		1, 1, -1, -1,
		0, 0, 0, 0,
		1, -1, 1, -1
	};
	const float GRAD_Z[] = {
		0, 0, 0, 0,
		1, 1, -1, -1,
		1, 1, -1, -1
	};
	
	void init(int seed) {
		set_seed(seed);
	}
	
	void set_seed(int seed) {
		m_seed = seed;

		std::mt19937 gen(seed);

		for (int i = 0; i < 256; i++)
			m_perm[i] = (unsigned char)i;

		for (int j = 0; j < 256; j++)
		{
			int rng = (int)(gen() % (256 - j));
			int k = rng + j;
			int l = m_perm[j];
			m_perm[j] = m_perm[j + 256] = m_perm[k];
			m_perm[k] = (unsigned char)l;
			m_perm12[j] = m_perm12[j + 256] = m_perm[j] % 12;
		}
	}
		
	static inline unsigned char Index2D_12(unsigned char offset, int x, int y){
		return m_perm12[(x & 0xff) + m_perm[(y & 0xff) + offset]];
	}
	
	static inline float GradCoord2D(unsigned char offset, int x, int y, float xd, float yd){
		unsigned char lutPos = Index2D_12(offset, x, y);
		return xd*GRAD_X[lutPos] + yd*GRAD_Y[lutPos];
	}
			
	static float SinglePerlin(unsigned char offset, float x, float y) {
		int x0 = FastFloor(x);
		int y0 = FastFloor(y);
		int x1 = x0 + 1;
		int y1 = y0 + 1;

		float xs = 0, ys = 0;
		switch (m_interp)
		{
		case Linear:
			xs = x - (float)x0;
			ys = y - (float)y0;
			break;
		case Hermite:
			xs = InterpHermiteFunc(x - (float)x0);
			ys = InterpHermiteFunc(y - (float)y0);
			break;
		case Quintic:
			xs = InterpQuinticFunc(x - (float)x0);
			ys = InterpQuinticFunc(y - (float)y0);
			break;
		}

		float xd0 = x - (float)x0;
		float yd0 = y - (float)y0;
		float xd1 = xd0 - 1;
		float yd1 = yd0 - 1;

		float xf0 = Lerp(GradCoord2D(offset, x0, y0, xd0, yd0), GradCoord2D(offset, x1, y0, xd1, yd0), xs);
		float xf1 = Lerp(GradCoord2D(offset, x0, y1, xd0, yd1), GradCoord2D(offset, x1, y1, xd1, yd1), xs);

		return Lerp(xf0, xf1, ys);
	}
		
	static const float SQRT3 = float(1.7320508075688772935274463415059);
	static const float F2 = float(0.5) * (SQRT3 - float(1.0));
	static const float G2 = (float(3.0) - SQRT3) / float(6.0);
	static float SingleSimplex(unsigned char offset, float x, float y) {
		float t = (x + y) * F2;
		int i = FastFloor(x + t);
		int j = FastFloor(y + t);

		t = (i + j) * G2;
		float X0 = i - t;
		float Y0 = j - t;

		float x0 = x - X0;
		float y0 = y - Y0;

		int i1, j1;
		if (x0 > y0)
		{
			i1 = 1; j1 = 0;
		}
		else
		{
			i1 = 0; j1 = 1;
		}

		float x1 = x0 - (float)i1 + G2;
		float y1 = y0 - (float)j1 + G2;
		float x2 = x0 - 1 + 2*G2;
		float y2 = y0 - 1 + 2*G2;

		float n0, n1, n2;

		t = float(0.5) - x0*x0 - y0*y0;
		if (t < 0) n0 = 0;
		else
		{
			t *= t;
			n0 = t * t * GradCoord2D(offset, i, j, x0, y0);
		}

		t = float(0.5) - x1*x1 - y1*y1;
		if (t < 0) n1 = 0;
		else
		{
			t *= t;
			n1 = t*t*GradCoord2D(offset, i + i1, j + j1, x1, y1);
		}

		t = float(0.5) - x2*x2 - y2*y2;
		if (t < 0) n2 = 0;
		else
		{
			t *= t;
			n2 = t*t*GradCoord2D(offset, i + 1, j + 1, x2, y2);
		}

		return 70 * (n0 + n1 + n2);
	}
	
	float perlin(float x, float y) {
		return SinglePerlin(0, x * m_frequency, y * m_frequency);
	}
	
	float simplex(float x, float y) {
		return SingleSimplex(0, x * m_frequency, y * m_frequency);
	}
}

namespace Tiling {
    unsigned tilemap_index(const TileMap &tile_map, const unsigned layer, const unsigned x, const unsigned y) {
        int layer_start = layer * tile_map.columns * tile_map.rows;
        return layer_start + y*tile_map.columns+x;
    }

    void tilemap_load(const std::string file, TileMap &tile_map) {
        std::string map_name = Engine::get_base_data_folder() + file;
        std::ifstream tile_map_stream(map_name);
        
        if(tile_map_stream) {
            tile_map_stream >> tile_map.layers;
            tile_map_stream >> tile_map.columns;
            tile_map_stream >> tile_map.rows;
            tile_map_stream >> tile_map.tile_size;
            unsigned tile_count = tile_map.layers * tile_map.columns * tile_map.rows;
            tile_map.tiles = new unsigned[tile_count];
            for(unsigned layer = 1; layer <= tile_map.layers; layer++) {
                for(unsigned i = 0; i < tile_map.columns * tile_map.rows; i++) {
                    int tile;
                    tile_map_stream >> tile;
                    tile_map.tiles[tile_map.layers * i] = tile;
                }
            }
        } else {
            Engine::log("\n[WARNING] unable to open tilemap file");
        }
    }

	void tilemap_make(TileMap &tile_map, unsigned layers, unsigned columns, unsigned rows, unsigned tile_size, unsigned default_tile) {
        tile_map.layers = layers;
        tile_map.columns = columns;
        tile_map.rows = rows;
        tile_map.tile_size = tile_size;
        unsigned tile_count = tile_map.layers * tile_map.columns * tile_map.rows;
		Engine::log("tile_count: %d", tile_count);
        tile_map.tiles = new unsigned[tile_count];
        for(unsigned layer = 1; layer <= tile_map.layers; layer++) {
            for(unsigned i = 0; i < tile_map.columns * tile_map.rows; i++) {
                tile_map.tiles[tile_map.layers * i] = default_tile;
            }
        }
    }
}

namespace Sound {
	struct PlayMessage { 
		SoundId id;
  		int volume;
	};

	const int MAX_QUEUE_SIZE = 16;
	static int queue_head;
  	static int queue_tail;
  	static PlayMessage play_queue[MAX_QUEUE_SIZE];

	SoundId load(const std::string &file_name) {
		std::string path = Engine::get_base_data_folder() + "sound/" + file_name;
		sound_load(path);
		return  0;
	}

	void init() {
    	queue_head = 0;
    	queue_tail = 0;
		sound_init();
  	}

	void quit() {
		sound_exit();
	}

	void play_all() {
		while(queue_head != queue_tail) {
			play_next();
		}
	}

	void play_next() {
		// If there are no pending requests, do nothing.
		if (queue_head == queue_tail) {
			return;
		}

		// Engine::logn("Playing sound: %d with volume: %d", play_queue[queue_head].id, play_queue[queue_head].volume);
		sound_play(play_queue[queue_head].id, play_queue[queue_head].volume);
		
		queue_head = (queue_head + 1) % MAX_QUEUE_SIZE;
	}

	void queue(SoundId id, int volume) {
		ASSERT_WITH_MSG((queue_tail + 1) % MAX_QUEUE_SIZE != queue_head, "TOO MANY SOUNDS IN QUEUE, up the size!");
		
		// Walk the pending requests.
		for (int i = queue_head; i != queue_tail; i = (i + 1) % MAX_QUEUE_SIZE) {
			if (play_queue[i].id == id) {
				// Use the larger of the two volumes.
				play_queue[i].volume = Math::max_i(volume, play_queue[i].volume);

				// Don't need to enqueue.
				return;
			}
		}

  		// Add to the end of the list.
		play_queue[queue_tail].id = id;
		play_queue[queue_tail].volume = volume;
		queue_tail = (queue_tail + 1) % MAX_QUEUE_SIZE;
	}
};


/*
void eventqueue_Init(EventQueue *e) {
    e->listener_count = 0;
    e->queue_count = 0;
}

void eventqueue_RemoveListener(EventQueue *e, void (*fp)(int, std::shared_ptr<void>)) {
    for(int i = 0; i < e->listener_count; i++) {
        if(e->listener_list[i] == fp) {
            e->listener_count--;
            e->listener_list[i] = e->listener_list[e->listener_count];
        }
    }
}

void eventqueue_AddListener(EventQueue *e, void (*fp)(int, std::shared_ptr<void>)) {
    eventqueue_RemoveListener(e, fp);
    e->listener_list[e->listener_count++] = fp;
}

void eventqueue_TriggerEvent(EventQueue *e, const int eventId, std::shared_ptr<void> data) {
    for(int i = 0; i < e->listener_count; i++) {
        e->listener_list[i](eventId, data);
    }
}

void eventqueue_QueueEvent(EventQueue *e, const int eventId, std::shared_ptr<void> data) {
    GameEvent ge;
    ge.id = eventId;
    ge.data = data;
    e->queue[e->queue_count++] = ge;
}

void eventqueue_PumpQueuedEvents(EventQueue *e) {
    for(int i = 0; i < e->queue_count; i++) {
        auto ge = &e->queue[i];
        eventqueue_TriggerEvent(e, ge->id, ge->data);
        ge->id = -1;
        ge->data = nullptr;
    }
    e->queue_count = 0;
}

namespace GlobalEvents {
    static EventQueue *globalEvents;

    void init() {
        globalEvents = new EventQueue;
        eventqueue_Init(globalEvents);
    }

    void update() {
        eventqueue_PumpQueuedEvents(globalEvents);
    }

    EventQueue *getGlobalEventQueue() {
        return globalEvents;
    }
}
*/
/*
namespace Animation {
	std::shared_ptr<AnimationComponent> make() {
		auto ac = std::make_shared<AnimationComponent>();
		ac->current_animation = ac->animatons.end();
		return ac;
	}

	std::shared_ptr<AnimationComponent> make(std::string name, std::string sprite_sheet, std::vector<SDL_Rect> frames, float fps, bool loop) {
		auto ac = std::make_shared<AnimationComponent>();
		add(ac, name, sprite_sheet, frames, fps, loop);
		return ac;
	}

	void set_current(std::shared_ptr<AnimationComponent> ac, std::string animation) {
		ac->current_animation = ac->animatons.find(animation);
	}

	void add(std::shared_ptr<AnimationComponent> ac, std::string name, std::string sprite_sheet, std::vector<SDL_Rect> frames, float fps, bool loop) {
		std::shared_ptr<Animation> a = std::make_shared<Animation>();
		a->sprite_sheet = sprite_sheet;
		a->frames = frames;
		a->fps = fps;
		a->duration = 1.0f / fps;
		a->loop = loop;
		ac->animatons.insert(std::pair<std::string, std::shared_ptr<Animation>>(name, a));

		set_current(ac, name);
	}

	void update(std::shared_ptr<AnimationComponent> a, float dt) {
		auto animation = a->current_animation->second;
		animation->timer += dt;
		if(animation->timer >= animation->duration) {
			animation->frame++;
			if(animation->frame >= (int)animation->frames.size()) {
				if(animation->loop) {
					animation->frame = 0;
				} else {
					animation->frame = animation->frames.size() - 1;
				}
			}
			animation->timer = 0;
		}
	}
	
	void get_sprite_sheet(const std::string name, const std::shared_ptr<AnimationComponent> ac, std::string &sprite_sheet) {
		sprite_sheet = ac->animatons[name]->sprite_sheet;
	}
	
	void get_frame_size(const std::shared_ptr<AnimationComponent> a, int &w, int &h) {
		auto animation = a->current_animation->second;
		w = (int)animation->frames[animation->frame].w;
		h = (int)animation->frames[animation->frame].h;
	}
}

namespace Easing {
	static Easing_I *easings = NULL;

	void init(int buffer_size) {
		easings = new Easing_I[buffer_size];
	}
	
	void start(int id, int *target, int end, float duration, easing_t ease_func) {
		ASSERT_WITH_MSG(easings != NULL, "easings not initialized, did you forget to init?");
		
		easings[id].ta.target = target;
		easings[id].ta.start = *target;
		easings[id].ta.end = end;
		easings[id].timer = 0.0f;
		easings[id].duration = duration;
		easings[id].ease_func = ease_func;
	}

	static void update_target(TargetData &d, float percent, easing_t ease_func) {
		float iv = Math::interpolate((float)d.start, (float)d.end, percent, ease_func);
		*d.target = static_cast<int>(iv);
	}
	
	bool completed(int id, float dt) {
		easings[id].timer += dt;
		if(easings[id].timer >= easings[id].duration)
			return true;
		
		update_target(easings[id].ta, easings[id].timer/easings[id].duration, easings[id].ease_func);
		return false;
	}
	
	bool ease_to(float *target, float start, float end, float duration, easing_t ease_func, float *timer, float dt) {
		static TargetData_f t;
		t.target = target;
		t.start = start;
		t.end = end;
		
		*timer += dt;
		if(*timer > duration)
			return true;

		float percent = *timer/duration;
		*t.target = Math::interpolate(t.start, t.end, percent, ease_func);
		return false;
	}
	
	void ping_pong(float *target, float start, float end, float duration, easing_t ease_func, float *timer, float dt) {
		static TargetData_f t;
		t.target = target;
		t.start = start;
		t.end = end;
		
		*timer += dt;
		if(*timer > duration*2)
			*timer = 0;
		
		float time = *timer > duration ? duration - (*timer - duration) : *timer;
		*t.target = Math::interpolate(t.start, t.end, time/duration, ease_func);
	}

	std::shared_ptr<Easing_F> make_float(float *target, float start, float end, float duration, easing_t ease_func) {
		auto easing = std::make_shared<Easing_F>();
		easing->timer = 0;
		easing->target = target;
		easing->start = start;
		easing->end = end;
		easing->duration = duration;
		easing->ease_func = ease_func;
		return easing;
	}

	bool update(std::shared_ptr<Easing_F> easing, float dt) {
		easing->timer += dt;
		if(easing->timer > easing->duration)
			return true;

		float percent = easing->timer/easing->duration;
		*easing->target = Math::interpolate(easing->start, easing->end, percent, easing->ease_func);
		return false;
	}
	
	std::shared_ptr<Easing_Point> make_point(Engine::Point *target, Engine::Point end, float duration, easing_t ease_func) {
		auto easing = std::make_shared<Easing_Point>();
		easing->timer = 0;
		easing->target = target;
		easing->start = Engine::Point(target->x, target->y);
		easing->end = end;
		easing->duration = duration;
		easing->ease_func = ease_func;
		return easing;
	}

	bool update(std::shared_ptr<Easing_Point> easing, float dt) {
		easing->timer += dt;
		//Engine::log("\ntimer: %f \t duration: %f", easing->timer, easing->duration);
		if(easing->timer > easing->duration)
			return true;

		float percent = easing->timer/easing->duration;
		Math::interpolate_point(easing->target, easing->start, easing->end, percent, easing->ease_func);
		return false;
	}

	std::shared_ptr<Easing_Vec2> make_vector2(Engine::Vector2 *target, Engine::Vector2 start, Engine::Vector2 end, float duration, easing_t ease_func) {
		auto easing = std::make_shared<Easing_Vec2>();
		easing->timer = 0;
		easing->target = target;
		easing->start = start;
		easing->end = end;
		easing->duration = duration;
		easing->ease_func = ease_func;
		return easing;
	}

	bool update(std::shared_ptr<Easing_Vec2> easing, float dt) {
		easing->timer += dt;
		//Engine::log("\ntimer: %f \t duration: %f", easing->timer, easing->duration);
		if(easing->timer > easing->duration)
			return true;

		float percent = easing->timer/easing->duration;
		Math::interpolate_vector(easing->target, easing->start, easing->end, percent, easing->ease_func);
		return false;
	}	
}

namespace Actions {
	std::shared_ptr<Action> make_ease(float *target, float start, float end, float duration, easing_t ease_func) {
		auto action = std::make_shared<Action>();
		action->duration = duration;
		action->update = [target, start, end, ease_func](float dt, std::shared_ptr<Action> action) {
			if(action->done)
				return;
			action->done = Easing::ease_to(target, start, end, action->duration, ease_func, &action->timer, dt);
		};
		return action;
	}
	
	std::shared_ptr<Action> make_ease_vector(Engine::Vector2 *target, Engine::Vector2 start, Engine::Vector2 end, float duration, easing_t ease_func) {
		auto action = std::make_shared<Action>();
		action->duration = duration;
		auto easing = Easing::make_vector2(target, start, end, duration, ease_func);
		action->update = [easing, ease_func](float dt, std::shared_ptr<Action> action) {
			if(action->done)
				return;
			action->done = Easing::update(easing, dt);
		};
		return action;
	}

	std::shared_ptr<Action> make_func(std::function<void(void)> func) {
		auto action = std::make_shared<Action>();
		action->update = [func](float dt, std::shared_ptr<Action> action) {
			if(action->done)
				return;
			func();
			action->done = true;			
		};
		return action;
	}

	Action_ptr make_delay(float delay) {
		auto action = std::make_shared<Action>();
		action->duration = delay;
		action->timer = 0;
		action->update = [](float dt, std::shared_ptr<Action> action) {
			if(action->done)
				return;
			action->timer += dt;
			if(action->timer > action->duration)
				action->done = true;
		};
		return action;
	}

	Action_ptr make_parallel(Action_ptr a, Action_ptr b) {
		auto action = std::make_shared<Action>();
		action->update = [a, b](float dt, std::shared_ptr<Action> action) {
			if(action->done)
				return;
			a->update(dt, a);
			b->update(dt, b);
			action->done = a->done && b->done;
		};
		return action;
	}
}
*/