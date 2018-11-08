#include "particles.h"
#include "renderer.h"

namespace Particles {
    void emit(ParticleContainer &c, const Emitter &p_config) {
		int particle_count = RNG::range_i(p_config.min_particles, p_config.max_particles);
		for(int i = 0; i < particle_count; ++i) {
			float life = RNG::range_f(p_config.life_min, p_config.life_max);
			float angle = RNG::range_f(p_config.angle_min, p_config.angle_max);
			float speed = RNG::range_f(p_config.speed_min, p_config.speed_max);
			float size = RNG::range_f(p_config.size_min, p_config.size_max);
			float size_end = RNG::range_f(p_config.size_end_min, p_config.size_end_max);
			spawn(c, p_config.position, life, angle, speed, size, size_end, p_config.force, p_config.color_start, p_config.color_end);
		}
	}
	
    void spawn(ParticleContainer &c, const Vector2 &p, const float &life, const float &angle, const float &speed, const float &size, const float &size_end, const Vector2 &force, const SDL_Color &color_start, const SDL_Color &color_end) {
		Particle &particle = c.particles[c.length++];
		particle.position = p;
		particle.life = life;
		particle.size = size;
		particle.size_shift = (size_end - size) / life; 
		particle.force = force;
		float angle_radians = angle * Math::Pi / 180;
		particle.velocity = Vector2(
			speed * cosf(angle_radians),
			speed * -sinf(angle_radians)
		);

		particle.color[0] = color_start.r;
		particle.color[1] = color_start.g;
		particle.color[2] = color_start.b;
		particle.color[3] = color_start.a;
		particle.color_shift[0] = (color_end.r - color_start.r) / life;
		particle.color_shift[1] = (color_end.g - color_start.g) / life;
		particle.color_shift[2] = (color_end.b - color_start.b) / life;
		particle.color_shift[3] = (color_end.a - color_start.a) / life;
	}

	void update(ParticleContainer &c, const float dt) {
		Particle *particles = c.particles;
		for(int i = 0; i < c.length; i++) {
            particles[i].life -= dt;
			if(particles[i].life < 0) {
				particles[i] = particles[--c.length];
			}

			particles[i].velocity += particles[i].force * dt;
			particles[i].position += particles[i].velocity * dt;

			particles[i].size += particles[i].size_shift * dt;

    		particles[i].color[0] += particles[i].color_shift[0] * dt;
			particles[i].color[1] += particles[i].color_shift[1] * dt;
			particles[i].color[2] += particles[i].color_shift[2] * dt;
			particles[i].color[3] += particles[i].color_shift[3] * dt;

			particles[i].color[0] = Math::clamp_f(particles[i].color[0], 0, 255);
			particles[i].color[1] = Math::clamp_f(particles[i].color[1], 0, 255);
			particles[i].color[2] = Math::clamp_f(particles[i].color[2], 0, 255);
			particles[i].color[3] = Math::clamp_f(particles[i].color[3], 0, 255);
		}
	}

    void render_circles(const ParticleContainer &c) {
		const Particle *particles = c.particles;
        for(int i = 0; i < c.length; i++) {
            draw_g_circle_RGBA((int)particles[i].position.x, 
                (int)particles[i].position.y, 
                (int)particles[i].size, 
                (uint8_t)particles[i].color[0],
                (uint8_t)particles[i].color[1],
                (uint8_t)particles[i].color[2],
                (uint8_t)particles[i].color[3]);
        }
    }

	void render_circles_filled(const ParticleContainer &c) {
		const Particle *particles = c.particles;
        for(int i = 0; i < c.length; i++) {
            draw_g_circle_filled_RGBA((int)particles[i].position.x, 
                (int)particles[i].position.y, 
                (int)particles[i].size, 
                (uint8_t)particles[i].color[0],
                (uint8_t)particles[i].color[1],
                (uint8_t)particles[i].color[2],
                (uint8_t)particles[i].color[3]);
        }
    }

	void render_rectangles_filled(const ParticleContainer &c) {
		const Particle *particles = c.particles;
		for(int i = 0; i < c.length; i++) {
			draw_g_rectangle_filled_RGBA((int)particles[i].position.x, 
                (int)particles[i].position.y, 
                (int)particles[i].size, 
				(int)particles[i].size,
                (uint8_t)particles[i].color[0],
                (uint8_t)particles[i].color[1],
                (uint8_t)particles[i].color[2],
                (uint8_t)particles[i].color[3]);
		}
	}

/*
    var startColor = [
    	this.startColor[0] + this.startColorVar[0] * random11(), 
    	this.startColor[1] + this.startColorVar[1] * random11(), 
    	this.startColor[2] + this.startColorVar[2] * random11(), 
    	this.startColor[3] + this.startColorVar[3] * random11()
    ];
    
    // and an ending color
    var endColor = [
    	this.endColor[0] + this.endColorVar[0] * random11(),
    	this.endColor[1] + this.endColorVar[1] * random11(),
    	this.endColor[2] + this.endColorVar[2] * random11(),
    	this.endColor[3] + this.endColorVar[3] * random11()
    ];
    
    // then figure out how much to shift the color each frame
    particle.color = startColor;
    particle.deltaColor = [
    	(endColor[0] - startColor[0]) / particle.life,
    	(endColor[1] - startColor[1]) / particle.life,
    	(endColor[2] - startColor[2]) / particle.life,
    	(endColor[3] - startColor[3]) / particle.life
	];

	in update:
	particle.color[0] += particle.deltaColor[0] * delta;
    particle.color[1] += particle.deltaColor[1] * delta;
    particle.color[2] += particle.deltaColor[2] * delta;
	particle.color[3] += particle.deltaColor[3] * delta;

	*/
}