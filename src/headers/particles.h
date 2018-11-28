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

	struct ParticleContainer {
		Particle *particles;
    	int length = 0;
    	size_t length_max = 0;
	};

	inline ParticleContainer make(size_t max_particles) {
		ParticleContainer p;
		p.particles = new Particle[max_particles];
		p.length = 0;
		p.length_max = max_particles;
		return p;
	}

    void emit(ParticleContainer &c, const Emitter &emitter);
    void spawn(ParticleContainer &c, const Vector2 &p, const float &life, const float &angle, const float &speed, const float &size, const float &size_end, const Vector2 &force, const SDL_Color &color_start, const SDL_Color &color_end);
    void update(ParticleContainer &c, const float dt);
	void clear(ParticleContainer &c);
	void render_circles(const ParticleContainer &c);
	void render_circles_filled(const ParticleContainer &c);
    void render_rectangles_filled(const ParticleContainer &c);

};

#endif