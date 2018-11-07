#ifndef PARTICLES_H
#define PARTICLES_H

#include "engine.h"

namespace Particles {
	struct Particle {
		Vector2 position;
		Vector2 velocity;
		Vector2 force;
		float color_shift[4];
		float color[4];
		float angle;
		float life;
		float size;
		float size_shift;
	};

	struct Emitter {
		Vector2 position;
		SDL_Color color_start;
		SDL_Color color_end;
		Vector2 force;
		int min_particles = 0;
		int max_particles = 0;
		float life_min = 0;
		float life_max = 0;
		float angle_min = 0;
		float angle_max = 0;
		float speed_min = 0;
		float speed_max = 0;
		float size_min = 0;
		float size_max = 0;
		float size_end_max = 0;
		float size_end_min = 0;
	};

    void init(size_t max_particles);
    void emit(const Emitter &emitter);
    void spawn(const Vector2 &p, const float &life, const float &angle, const float &speed, const float &size, const float &size_end, const Vector2 &force, const SDL_Color &color_start, const SDL_Color &color_end);
    void update(const float dt);
    void render();
    /*
	std::unique_ptr<Emitter> make(const ParticleConfig &config);
	void configure(std::unique_ptr<Emitter> &emitter, const ParticleConfig &config);
	void set_position(std::unique_ptr<Emitter> &emitter, const Engine::Vector2 &p);
	void emit(std::unique_ptr<Emitter> &emitter);
	void update(std::unique_ptr<Emitter> &emitter, const float dt);
	void render(std::unique_ptr<Emitter> &emitter);
	void spawn(std::unique_ptr<Emitter> &emitter, const Engine::Vector2 &p, const float &life, const float &angle, const float &speed, const float &size, const float &size_end, const Engine::Vector2 &force, const SDL_Color &color_start, const SDL_Color &color_end);
    */

	// void spawn(const Engine::Vector2 p, const float life, const float angle, const float speed, const float size, const Engine::Vector2 force, const SDL_Color color);
	// void spawn_config(ParticleConfig p_config);
	// void update(float dt);
	// void render();
};

#endif