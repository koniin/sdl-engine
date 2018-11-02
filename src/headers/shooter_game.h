
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
* Leave something behind when something is killed
    - destroy hit enemy
    - respawn after some time at random pos on screen
    - debris at spawn site that doesn't move away - how it looks does not matter now
* BIG random explosion/s on kill (circle that flashes from black/grey to white to disappear for one update each)
* Shells or something fly out on fire weapon (make it a "machine gun")
    - check the clip from dropbox

* BIG random explosions

* Smoke on explosion
* Smoke on fire gun

When solid objects that are unbreakable:
* Impact effect (hit effect, like a little marker on the side we hit)

Do movement and then:
* Area larger than the screen with camera
* Camera lerp - follow player
* Camera towards where player is aiming
* Camera kick - move camera back on firing (moves back to player automatically if following)

Then:
* Sound and animatons
[ ] Player ship rotation animation (exists in sheet)
* More base in sound effects

* Gun gfx
* Gun kick - make it smaller or something when firing

*/

#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

void load_shooter();
void update_shooter();
void render_shooter();
void render_shooter_ui();


#endif