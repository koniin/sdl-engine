#include "engine.h"
#include "renderer.h"
#include "game.h"

#include <iostream>

gameTimer timer;

void windowEvent(const SDL_Event * event);

static SDL_Event event;

void input() {
	Input::update_states();
	while (SDL_PollEvent(&event)) {
		Input::map(&event);
		
		switch(event.type) {
			case SDL_QUIT:
				Engine::exit();
				break;
			case SDL_WINDOWEVENT: {
				// windowEvent(&event);
        		break;             
			} 
			case SDL_KEYDOWN: {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					Engine::exit();
				} else if (event.key.keysym.sym == SDLK_F1) {
					window_set_scale(1);
					window_center();
				} else if (event.key.keysym.sym == SDLK_F2) {
					window_set_scale(2);
					window_center();
				} else if (event.key.keysym.sym == SDLK_F3) {
					window_set_scale(3);
					window_center();
				} else if (event.key.keysym.sym == SDLK_F4) {
					window_set_scale(3);
					window_toggle_fullscreen(false);
				} else if (event.key.keysym.sym == SDLK_F5) {
					window_set_scale(3);
					window_toggle_fullscreen(true);
				}
				break;
			}
		}
	}
}


int main(int argc, char* argv[]) {
	if(!renderer_init("TITLE", 640, 360, 2)) {
		printf("init renderer failed");
		return 1;
	}

	Input::init();

	game_load();
	
	// Initiate timer
    timer.now = SDL_GetPerformanceCounter();
    timer.last = 0;
    timer.dt = 0;
    timer.fixed_dt = 1.0/60.0;
    timer.accumulator = 0;

	// FPS timer
	Uint32 fps_lasttime = SDL_GetTicks(); //the last recorded time.
	Uint32 fps_current; //the current FPS.
	Uint32 fps_frames = 0; //frames passed since the last recorded fps.

	Time::deltaTime = (float)timer.fixed_dt;

    while (Engine::is_running()) {
		timer.last = timer.now;
        timer.now = SDL_GetPerformanceCounter();
        timer.dt = ((timer.now - timer.last)/(double)SDL_GetPerformanceFrequency());
        // This timing method is the 4th (Free the Physics) from this article: https://gafferongames.com/post/fix_your_timestep/
        timer.accumulator += timer.dt;
		
        while (timer.accumulator >= timer.fixed_dt) {	
			input();
			game_update();
            timer.accumulator -= timer.fixed_dt;
        }
		
		game_render();

		fps_frames++;
#define FPS_INTERVAL 1.0 //seconds.
		if (fps_lasttime < SDL_GetTicks() - FPS_INTERVAL*1000)
		{
			fps_lasttime = SDL_GetTicks();
			fps_current = fps_frames;
			fps_frames = 0;
			printf("\nFPS: %d", fps_current);
		}
	}
	
	game_unload();

	renderer_destroy();
	
    return 0;
}

void windowEvent(const SDL_Event *window_event) {
    switch (window_event->window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            SDL_Log("Window %d shown", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            SDL_Log("Window %d hidden", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_Log("Window %d exposed", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            SDL_Log("Window %d moved to %d,%d",
                    window_event->window.windowID, window_event->window.data1,
                    window_event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            SDL_Log("Window %d resized to %dx%d",
                    window_event->window.windowID, window_event->window.data1,
                    window_event->window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    window_event->window.windowID, window_event->window.data1,
                    window_event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            SDL_Log("Window %d minimized", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            SDL_Log("Window %d restored", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            SDL_Log("Mouse entered window %d",
                    window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            SDL_Log("Mouse left window %d", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus",
                    window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus",
                    window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", window_event->window.windowID);
            break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            SDL_Log("Window %d is offered a focus", window_event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            SDL_Log("Window %d has a special hit test", window_event->window.windowID);
            break;
#endif
        default:
            SDL_Log("Window %d got unknown event %d",
                    window_event->window.windowID, window_event->window.event);
            break;
        }
}

	// const float dt = 16.666; // 60 updates/s
	// float currentTime = SDL_GetTicks();
	// float accumulator = 0.0;

	// while (running) {
		// while (SDL_PollEvent(&event)) {
			// if (event.type == SDL_QUIT) running = false;
		// }
		
		// Uint32 newTime = SDL_GetTicks();
		// float frameTime = newTime - currentTime;
		// const Uint32 maxFrameTime = 1000; // 1 sec per frame is the slowest we allow
		// if (frameTime > maxFrameTime)
			// frameTime = maxFrameTime;
		
		// short count = 0;
		
		// currentTime = newTime;
		// accumulator += frameTime;
		// while (accumulator >= dt) {
			// count++;
		    // accumulator -= dt;
		// }
		
		// if(count > 1)
			// printf("double trouble");
		// else 
			// printf("no ");
		
		// GPU_Clear(screen);
		
		// Scenes::render();
		
		// GPU_Flip(screen);
		
		// frameCount++;
        // if(frameCount%500 == 0)
			// printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
		
		// SDL_Delay(1);
	// }
	
	