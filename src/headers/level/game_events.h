#ifndef GAME_EVENTS_H
#define GAME_EVENTS_H

#include "engine.h"
#include <stack>
#include <vector>
#include <unordered_map>
#include <array>

struct GEvent {
	size_t type;
	static size_t counter;

	template<typename T>
	bool is() {
		return getType<T>() == type;
	}

	template<typename T>
	void set() {
		type = getType<T>();
	}

	template<typename T>
	T *get() {
		return static_cast<T*>(this);
	}
	
	template<typename T>
	static size_t getType() {
		static size_t id = counter++;
		return id;
	}
};

struct PlayerFireBullet : GEvent {
	int test = 21;
};

namespace GameEvents {
	struct BaseContainer {};
	template<typename T>
	struct StackContainer : BaseContainer {
		std::stack<T*> stack;
	};
	
	static std::vector<GEvent*> event_queue;
	static std::array<BaseContainer*, 128> array_map;
	
	template<typename T>
	void init_stack(int sz) {
		auto s = new StackContainer<T>();
		for(int i = 0; i < sz; i++)
			s->stack.push(new T());	
		auto type = GEvent::getType<T>();
		array_map[type] = s;
	}

	inline void init(int sz) {
		event_queue.reserve(sz);

		init_stack<PlayerFireBullet>(sz);
	}

	template<typename T>
    void queue(T *event) {
        event->set<T>();
        event_queue.push_back(event);
		Engine::logn("Queueing event %d", event->type);
    }

    inline std::vector<GEvent*> &get_queued_events() {
		return event_queue;
	}

	template<typename T>
	T *get_event() {
		auto s = static_cast<StackContainer<T>*>(array_map[GEvent::getType<T>()]);
		auto r = s->stack.top();
		s->stack.pop();
		return r;

		// dynamic
		//return new T();
	}

	template<typename T>
	void return_event(T *event) {
		auto s = static_cast<StackContainer<T>*>(array_map[GEvent::getType<T>()]);
		s->stack.push(event);
		
		// dynamic
		// delete event;
	}

	inline void clear() {
		event_queue.clear();
	}
};

#endif