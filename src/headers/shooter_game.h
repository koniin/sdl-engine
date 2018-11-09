
// Game Feel TODO:
/* ==========================

All gfx can be found in shooter_spritesheet.png
[X] Bullet gfx (big)
[X] Ship gfx
[X] Enemy gfx
[X] Muzzle flash (circular filled white first frame or something or display bullet as circle first frame)
[X] Bullet spread (accuracy)
[X] Hit animation (Blink)
[X] Enemy knockback (3 pixels per frame in the direction of the bullet, would be countered by movement in normal cases)
[X] Screen shake on fire weapon
[X] Screen shake on hit enemy
[X] player knockback on fire weapon (if player is too far back move to start pos for demo)
[X] Sleep on hit an enemy (20ms)
[X] BIG random explosion/s on kill (circle that flashes from black/grey to white to disappear for one update each)

Do movement and then:
[X] Area larger than the screen with camera
[X] Camera lerp - follow player
[X] Camera towards where player is aiming
[X] Camera kick - move camera back on firing (moves back to player automatically if following)

[X] engine flame from ship(s) - or we do this with one sprite that changes shape 
    - steredenn uses fixed animation depending on speed
    - luftrausers uses fixed sprite that changes shape rapidly and only shows on forward movment

Particles
[X] Particle spray on explosion?
[ ] Smoke or something when firing from ship
[X] Small Particle spray on hit enemy (and not dead)
[ ] Particle trail from player when moving

[ ] Only show ship exhaust fire when moving

Then:
[ ] Player ship rotation animation (exists in sheet)
[ ] Sound - just test with a fire sound and hit sound
[ ] Animatons
    - Enemy => a round or squarish metal/black sprite with a blinking light

[ ] More base in sound effects

[ ] Particle follow bullet first few frames (for very very fast moving things like railgun/lazer?)

* Leave something behind when something is killed
    - destroy hit enemy
    - respawn after some time at random pos on screen
    - debris at spawn site that doesn't move away - how it looks does not matter now
    - nice probably -> pieces flies out at full color and then gradually gets darker and then at some point they stay floating

* Smoke on explosion - particles

When solid objects that are unbreakable:
* Impact effect (hit effect, like a little marker on the side we hit)

For GUNS
* Gun gfx
* Gun kick - make it smaller or something when firing
* Smoke on fire gun
[ ] Shells or something fly out on fire weapon (make it a "machine gun") - depends on bullets/gun

* BIG random explosions

*/

#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

#include "engine.h"
#include "renderer.h"

// Pixels per frame
constexpr float player_bullet_speed() {
    return 8.0f / 0.016667f;
}
constexpr float player_move_acceleration() {
    return 10.0f / 0.016667f;
}

struct PlayerConfiguration {
    int16_t radius = 8;
	float rotation_speed = 3.0f; // degrees
	float move_acceleration = player_move_acceleration();
	float drag = 0.04f;
	float fire_cooldown = 0.15f; // s
	float bullet_speed = player_bullet_speed();
    float gun_barrel_distance = 11.0f; // distance from center
    float fire_knockback = 2.0f; // pixels
    float fire_knockback_camera = -6.0f;
};

struct TargetConfiguration {
    float knockback_on_hit = 2.0f;
};

static const size_t RENDER_BUFFER_MAX = 256;

void load_shooter();
void update_shooter();
void render_shooter();
void render_shooter_ui();


#endif