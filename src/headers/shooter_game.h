
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
[ ] BIG random explosion/s on kill (circle that flashes from black/grey to white to disappear for one update each)
* Shells or something fly out on fire weapon (make it a "machine gun")
    - check the clip from dropbox

Do movement and then:
* Area larger than the screen with camera
* Camera lerp - follow player
* Camera towards where player is aiming
* Camera kick - move camera back on firing (moves back to player automatically if following)

* Try mouse aiming - like twin stick ;D

Then:
* Sound and animatons
[ ] Player ship rotation animation (exists in sheet)
* More base in sound effects

Particles
* Follow bullet first few frames
* as smoke or something when firing from ship
* engine flame from ship(s)
*  

* Leave something behind when something is killed
    - destroy hit enemy
    - respawn after some time at random pos on screen
    - debris at spawn site that doesn't move away - how it looks does not matter now

* Smoke on explosion

When solid objects that are unbreakable:
* Impact effect (hit effect, like a little marker on the side we hit)

For GUNS
* Gun gfx
* Gun kick - make it smaller or something when firing
* Smoke on fire gun

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