#include "renderer.h"
#include "SDL_image.h"
#include <fstream>

unsigned gw;
unsigned gh;
static unsigned step_scale;
static int window_w;
static int window_h;
int windowPos;

Font *default_font;
gfx renderer;

struct Camera {
	float shake_duration = 0.0f;
	float trauma = 0.0f;
	int x = 0;
	int y = 0;
	int offset_x = 0;
	int offset_y = 0;
} camera;

namespace Resources {
	std::unordered_map<std::string, Sprite*> sprites;
	std::unordered_map<std::string, Font*> fonts;
	
	static SDL_Texture* load_texture(const std::string &path, int &w, int &h) { 
		//The final texture 
		SDL_Texture* newTexture = NULL; 
		//Load image at specified path 
		SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); 
		if( loadedSurface == NULL ) { 
			printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() ); 
		} else { 
			//Create texture from surface pixels 
			newTexture = SDL_CreateTextureFromSurface(renderer.renderer, loadedSurface ); 
			if( newTexture == NULL ) { 
				printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() ); 
			} 

			w = loadedSurface->w;
			h = loadedSurface->h;

			//Get rid of old loaded surface 
			SDL_FreeSurface( loadedSurface ); 
		} 
		return newTexture; 
	}
	
    static SDL_Texture* load_streaming_texture(const std::string &path, int &w, int &h) {
		//The final texture 
		SDL_Texture* newTexture = NULL; 
		//Load image at specified path 
		SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); 
		if( loadedSurface == NULL ) { 
			printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() ); 
		} else { 
			SDL_Surface *formattedSurface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_ARGB8888, 0);
            newTexture = SDL_CreateTexture( renderer.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h );
			SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);
			if( newTexture == NULL ) { 
				printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() ); 
			} 
			w = formattedSurface->w; 
            h = formattedSurface->h;
			
            //Lock texture for manipulation 
            void* pixels; 
            int pitch; 
            SDL_LockTexture( newTexture, NULL, &pixels, &pitch ); 
            //Copy loaded/formatted surface pixels 
            memcpy( pixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h ); 

            //Unlock texture to update 
            SDL_UnlockTexture( newTexture ); 
            pixels = NULL; 

			//Get rid of old loaded surface 
			SDL_FreeSurface( loadedSurface ); 
            SDL_FreeSurface( formattedSurface );
		}

        return newTexture;
	}

    void transform_non_transparent_to_white(SDL_Texture* texture, int height) {
        void* texture_pixels; 
        int pitch; 
        SDL_LockTexture( texture, NULL, &texture_pixels, &pitch );
		
        //Uint32 format = SDL_GetWindowPixelFormat( renderer.sdl_window ); 
        SDL_PixelFormat* mappingFormat = SDL_AllocFormat( SDL_PIXELFORMAT_ARGB8888 ); 

        //Get pixel data 
        Uint32* pixels = (Uint32*)texture_pixels; 
        int pixelCount = ( pitch / 4 ) * height;

        //Map colors 
        Uint32 transparent = SDL_MapRGBA( mappingFormat, 0x0, 0x0, 0x0, 0x0 ); 
        Uint32 white = SDL_MapRGBA( mappingFormat, 0xFF, 0xFF, 0xFF, 0xFF ); 
		
        // Change transparent pixels to white
        for( int i = 0; i < pixelCount; ++i ) { 
            if(pixels[i] != transparent) {
                pixels[i] = white;
			} 
        } 

        //Unlock texture 
        SDL_UnlockTexture( texture ); 

        pixels = NULL; 
        pitch = 0;
    }

    Sprite *sprite_load(const std::string &name, const std::string &filename) {
		std::string path = Engine::get_base_data_folder() + filename;
		Sprite *s = new Sprite;
    	s->image = load_texture(path, s->w, s->h);
		sprites[name] = s;
		return s;
	}

	Sprite *sprite_load_white(const std::string &name, const std::string &filename) {
		std::string path = Engine::get_base_data_folder() + filename;
		Sprite *s = new Sprite;
    	s->image = load_streaming_texture(path, s->w, s->h);
		transform_non_transparent_to_white(s->image, s->h);
		sprites[name] = s;
		return s;
	}

    Sprite *sprite_get(const std::string &name) {
		return sprites.at(name);
	}
	
    void sprite_remove(const std::string &name) {
		auto itr = sprites.find(name);
		if (itr != sprites.end()) {
			SDL_DestroyTexture(itr->second->image);
    		delete itr->second;
    		sprites.erase(itr);
		}
	}

	void sprite_sheet_load(const std::string file, SpriteSheet &s) {
		std::string path = Engine::get_base_data_folder() + file;
		std::ifstream sprite_sheet_data(path);
		
		sprite_sheet_data >> s.sprite_sheet_name;

		Resources::sprite_load(s.sprite_sheet_name, s.sprite_sheet_name);

		if(sprite_sheet_data) {
			int sprite_count = 0;
			sprite_sheet_data >> sprite_count;
			for(int i = 0; i < sprite_count; i++) {
				SpriteFrame sf;
				sprite_sheet_data >> sf.id;
				sprite_sheet_data >> sf.name;
				SDL_Rect r;
				sprite_sheet_data >> r.x;
				sprite_sheet_data >> r.y;
				sprite_sheet_data >> r.w;
				sprite_sheet_data >> r.h;
				sf.region = r;

				int id = (int)s.sheet_sprites.size();
				s.sheet_sprites.push_back(sf);
				s.sprites_by_id[sf.id] = id;
				s.sprites_by_name[sf.name] = id;
			}
		}
	}

    Font *font_load(const std::string name, const std::string filename, int pointSize) {
		std::string path = Engine::get_base_data_folder() + filename;
		Font *f = new Font;
		TTF_Font *font = TTF_OpenFont(path.c_str(), pointSize);
		f->font = font;
		f->name = name;
		fonts[name] = f;
		return f;
	}

    Font *font_get(const std::string &name) {
		return fonts.at(name);
	}
	
	void font_set_style(const std::string &name, FontStyle style) {
		TTF_SetFontStyle(fonts.at(name)->font, style);
	}

	void font_set_outline(const std::string &name, int outline) {
		TTF_SetFontOutline(fonts.at(name)->font, outline);
	}

    void font_remove(const std::string& name) {
		auto itr = fonts.find(name);
		if (itr != fonts.end()) {
			TTF_CloseFont(itr->second->font);
    		delete itr->second;
    		fonts.erase(itr);
		}
	}

    void cleanup() {
		for(std::unordered_map<std::string, Sprite*>::iterator itr = sprites.begin(); itr != sprites.end(); itr++) {
			SDL_DestroyTexture(itr->second->image);
        	delete itr->second;
    	}
		sprites.clear();
		for(std::unordered_map<std::string, Font*>::iterator itr = fonts.begin(); itr != fonts.end(); itr++) {
			TTF_CloseFont(itr->second->font);
			delete itr->second;
    	}
		fonts.clear();

		TextCache::clear();
	}
}

namespace TextCache {
	std::unordered_map<size_t, Sprite> text_cache;
	std::hash<std::string> hasher;

	static size_t cache_key(Font *font, const SDL_Color &color, const char *text) {
		int colors = color.r | (color.g << 8) | (color.b << 16) | (color.a << 24);
		return hasher(font->name + text + std::to_string(colors));
	}

	Sprite &load(Font *font, const SDL_Color &color, const char *text) {
		size_t key = cache_key(font, color, text);

		auto item = text_cache.find(key);
		if(item == text_cache.end()) {
			Sprite ci;
			SDL_Surface *surface = TTF_RenderText_Solid(font->font, text, color);
			ci.image = SDL_CreateTextureFromSurface(renderer.renderer, surface);
			SDL_FreeSurface(surface);
			SDL_QueryTexture(ci.image, NULL, NULL, &ci.w, &ci.h);
			text_cache[key] = ci;
		} 

		return text_cache[key];
	}

	void clear() {
		for(auto &cache_item : text_cache) {
			SDL_DestroyTexture(cache_item.second.image);
		}
		text_cache.clear();
	}
}

void window_set_position(int x, int y) {
	SDL_SetWindowPosition(renderer.sdl_window, x, y);
}

void window_center() {
	window_set_position(windowPos, windowPos);
}

void window_set_title(const char* title) {
	SDL_SetWindowTitle(renderer.sdl_window, title);
}

void window_set_scale(const unsigned s) {
	if (s == step_scale)
		return;

	step_scale = s;
	window_w = (int)(gw * s);
	window_h = (int)(gh * s);
	
	SDL_SetWindowSize(renderer.sdl_window, window_w, window_h);
}

static bool is_fullscreen = false;
void window_toggle_fullscreen(bool useDesktopResolution) {
	//GPU_SetFullscreen(!GPU_GetFullscreen(), useDesktopResolution);
	if(is_fullscreen) {
		SDL_SetWindowFullscreen(renderer.sdl_window, 0);
		is_fullscreen = false;
		int ww, wh;
		SDL_GetWindowSize(renderer.sdl_window, &ww, &wh);
		window_w = ww;
		window_h = wh;
	} else {
		SDL_SetWindowFullscreen(renderer.sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		is_fullscreen = true;
		int ww, wh;
		SDL_GetWindowSize(renderer.sdl_window, &ww, &wh);
		window_w = ww;
		window_h = wh;
	}
}

void set_default_font(Font *font) {
	default_font = font;
}

void draw_sprite(const Sprite *sprite, int x, int y) {
	SDL_Rect destination_rect;
	destination_rect.x = x;
 	destination_rect.y = y;
  	destination_rect.w = sprite->w;
  	destination_rect.h = sprite->h;

	SDL_RenderCopy(renderer.renderer, sprite->image, NULL, &destination_rect);
}

void draw_sprite_centered(const Sprite *sprite, int x, int y) {
	int w = sprite->w;
	int h = sprite->h;
	SDL_Rect destination_rect;
	destination_rect.x = x - (w / 2);
 	destination_rect.y = y - (h / 2);
  	destination_rect.w = w;
  	destination_rect.h = h;

	SDL_RenderCopy(renderer.renderer, sprite->image, NULL, &destination_rect);
}

void draw_sprite_region(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y) {
	SDL_Rect destination_rect;
	destination_rect.x = x;
 	destination_rect.y = y;
  	destination_rect.w = src_rect->w;
  	destination_rect.h = src_rect->h;

	SDL_RenderCopy(renderer.renderer, sprite->image, src_rect, &destination_rect);
}

void draw_sprite_region_centered(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y) {
	int w = src_rect->w;
	int h = src_rect->h;
	SDL_Rect destination_rect;
	destination_rect.x = x - (w / 2);
 	destination_rect.y = y - (h / 2);
  	destination_rect.w = w;
  	destination_rect.h = h;

	SDL_RenderCopy(renderer.renderer, sprite->image, src_rect, &destination_rect);
}

void draw_sprite_region_centered_rotated(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y, float angle) {
	int w = src_rect->w;
	int h = src_rect->h;
	SDL_Rect destination_rect;
	destination_rect.x = x - (w / 2);
 	destination_rect.y = y - (h / 2);
  	destination_rect.w = w;
  	destination_rect.h = h;

	SDL_RenderCopyEx(renderer.renderer, sprite->image, src_rect, &destination_rect, angle, NULL, SDL_FLIP_NONE);
}

void draw_spritesheet_name(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y) {
	draw_sprite_region(Resources::sprite_get(s.sprite_sheet_name), &s.sheet_sprites[s.sprites_by_name.at(sprite)].region, x, y);
}

void draw_spritesheet_name_centered(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y) {
	draw_sprite_region_centered(Resources::sprite_get(s.sprite_sheet_name), &s.sheet_sprites[s.sprites_by_name.at(sprite)].region, x, y);
}

void draw_spritesheet_name_centered_rotated(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y, const float &angle) {
	draw_sprite_region_centered_rotated(Resources::sprite_get(s.sprite_sheet_name), &s.sheet_sprites[s.sprites_by_name.at(sprite)].region, x, y, angle);
}

void draw_text(int x, int y, const SDL_Color &color, const char *text) {
	draw_text_font(default_font, x, y, color, text);
}

void draw_text_str(int x, int y, const SDL_Color &color, const std::string text) {
	draw_text_font(default_font, x, y, color, text.c_str());
}

void draw_text_font(Font *font, int x, int y, const SDL_Color &color, const char *text) {
	Sprite &cacheItem = TextCache::load(font, color, text);
	draw_sprite(&cacheItem, x, y);
}

void draw_text_centered(int x, int y, const SDL_Color &color, const char *text) {
	draw_text_font_centered(default_font, x, y, color, text);
}

void draw_text_centered_str(int x, int y, const SDL_Color &color, std::string text) {
	draw_text_font_centered(default_font, x, y, color, text.c_str());
}

void draw_text_font_centered(Font *font, int x, int y, const SDL_Color &color, const char *text) {
	Sprite &cacheItem = TextCache::load(font, color, text);
	draw_sprite_centered(&cacheItem, x, y);
	// SDL_Rect destination_rect;
	// destination_rect.x = x - (cacheItem.w / 2);
 	// destination_rect.y = y - (cacheItem.h / 2);
	// destination_rect.w = cacheItem.w;
	// destination_rect.h = cacheItem.h;

	// SDL_RenderCopy(renderer.renderer, cacheItem.image, NULL, &destination_rect);
}

void draw_tilemap_ortho(const TileMap &t, const SpriteSheet &s, const int x_start, const int y_start) {
	for(unsigned layer = 0; layer < t.layers; layer++) {
		for(unsigned y = 0; y < t.rows; y++) {
			for(unsigned x = 0; x < t.columns; x++) {
				unsigned tile = t.tiles[Tiling::tilemap_index(t, layer, x, y)];
				draw_sprite_region(Resources::sprite_get(s.sprite_sheet_name), 
					&s.sheet_sprites[s.sprites_by_id.at(tile)].region,
					x_start + x * t.tile_size, y_start + y * t.tile_size);
			}
		}
	}
}

void draw_g_pixel(int16_t x, int16_t y) {
    SDL_RenderDrawPoint(renderer.renderer, x, y);
}

void draw_g_pixel_color(int16_t x, int16_t y, const SDL_Color &color) {
    draw_g_pixel_RGBA(x, y, color.r, color.g, color.b, color.a);
}

void draw_g_pixel_RGBA(int16_t x, int16_t y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawBlendMode(renderer.renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer.renderer, r, g, b, a);
    SDL_RenderDrawPoint(renderer.renderer, x, y);
}

void draw_g_horizontal_line(int16_t x1, int16_t x2, int16_t y) {
    SDL_RenderDrawLine(renderer.renderer, x1, y, x2, y);
}

void draw_g_horizontal_line_color(int16_t x1, int16_t x2, int16_t y, SDL_Color &color) {
	draw_g_horizontal_line_RGBA(x1, x2, y, color.r, color.g, color.b, color.a);
}

void draw_g_horizontal_line_RGBA(int16_t x1, int16_t x2, int16_t y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawBlendMode(renderer.renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer.renderer, r, g, b, a);
    SDL_RenderDrawLine(renderer.renderer, x1, y, x2, y);        
}

void draw_g_vertical_line_color(int16_t x, int16_t y1, int16_t y2, SDL_Color &color) {
	draw_g_vertical_line_RGBA(x, y1, y2, color.r, color.g, color.b, color.a);
}

void draw_g_vertical_line_RGBA(int16_t x, int16_t y1, int16_t y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawBlendMode(renderer.renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer.renderer, r, g, b, a);
    SDL_RenderDrawLine(renderer.renderer, x, y1, x, y2);
}

void draw_g_circe_color(int16_t x, int16_t y, int16_t rad, SDL_Color &color) {
    draw_g_ellipseRGBA(x, y, rad, rad, color.r, color.g, color.b, color.a);
}

void draw_g_circe_RGBA(int16_t x, int16_t y, int16_t rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    draw_g_ellipseRGBA(x, y, rad, rad, r, g, b, a);
}

#pragma warning(push)
#pragma warning(disable:4244)
void draw_g_ellipseRGBA(int16_t x, int16_t y, int16_t rx, int16_t ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        int ix, iy;
        int h, i, j, k;
        int oh, oi, oj, ok;
        int xmh, xph, ypk, ymk;
        int xmi, xpi, ymj, ypj;
        int xmj, xpj, ymi, ypi;
        int xmk, xpk, ymh, yph;

        /*
        * Sanity check radii 
        */
        if ((rx < 0) || (ry < 0)) {
            return;
        }

        /*
        * Special case for rx=0 - draw a vline 
        */
        if (rx == 0) {
            draw_g_vertical_line_RGBA(x, y - ry, y + ry, r, g, b, a);
			return;
        }
        /*
        * Special case for ry=0 - draw a hline 
        */
        if (ry == 0) {
            draw_g_horizontal_line_RGBA(x - rx, x + rx, y, r, g, b, a);
			return;
        }

        /*
        * Set color
        */
        SDL_SetRenderDrawBlendMode(renderer.renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer.renderer, r, g, b, a);

        /*
        * Init vars 
        */
        oh = oi = oj = ok = 0xFFFF;

        /*
        * Draw 
        */
        if (rx > ry) {
                ix = 0;
                iy = rx * 64;

                do {
                        h = (ix + 32) >> 6;
                        i = (iy + 32) >> 6;
                        j = (h * ry) / rx;
                        k = (i * ry) / rx;

                        if (((ok != k) && (oj != k)) || ((oj != j) && (ok != j)) || (k != j)) {
                                xph = x + h;
                                xmh = x - h;
                                if (k > 0) {
                                        ypk = y + k;
                                        ymk = y - k;
                                        draw_g_pixel(xmh, ypk);
                                        draw_g_pixel(xph, ypk);
                                        draw_g_pixel(xmh, ymk);
                                        draw_g_pixel(xph, ymk);
                                } else {
                                        draw_g_pixel(xmh, y);
                                        draw_g_pixel(xph, y);
                                }
                                ok = k;
                                xpi = x + i;
                                xmi = x - i;
                                if (j > 0) {
                                        ypj = y + j;
                                        ymj = y - j;
                                        draw_g_pixel(xmi, ypj);
                                        draw_g_pixel(xpi, ypj);
                                        draw_g_pixel(xmi, ymj);
                                        draw_g_pixel(xpi, ymj);
                                } else {
                                        draw_g_pixel(xmi, y);
                                        draw_g_pixel(xpi, y);
                                }
                                oj = j;
                        }

                        ix = ix + iy / rx;
                        iy = iy - ix / rx;

                } while (i > h);
        } else {
                ix = 0;
                iy = ry * 64;

                do {
                        h = (ix + 32) >> 6;
                        i = (iy + 32) >> 6;
                        j = (h * rx) / ry;
                        k = (i * rx) / ry;

                        if (((oi != i) && (oh != i)) || ((oh != h) && (oi != h) && (i != h))) {
                                xmj = x - j;
                                xpj = x + j;
                                if (i > 0) {
                                        ypi = y + i;
                                        ymi = y - i;
                                        draw_g_pixel(xmj, ypi);
                                        draw_g_pixel(xpj, ypi);
                                        draw_g_pixel(xmj, ymi);
                                        draw_g_pixel(xpj, ymi);
                                } else {
                                        draw_g_pixel(xmj, y);
                                        draw_g_pixel(xpj, y);
                                }
                                oi = i;
                                xmk = x - k;
                                xpk = x + k;
                                if (h > 0) {
                                        yph = y + h;
                                        ymh = y - h;
                                        draw_g_pixel(xmk, yph);
                                        draw_g_pixel(xpk, yph);
                                        draw_g_pixel(xmk, ymh);
                                        draw_g_pixel(xpk, ymh);
                                } else {
                                        draw_g_pixel(xmk, y);
                                        draw_g_pixel(xpk, y);
                                }
                                oh = h;
                        }

                        ix = ix + iy / ry;
                        iy = iy - ix / ry;

                } while (i > h);
        }
}
#pragma warning(pop)

void draw_g_rectangle_filled(int x, int y, int w, int h, const SDL_Color &color) {
	draw_g_rectangle_filled_RGBA(x, y, w, h, color.r, color.g, color.b, color.a);
}

void draw_g_rectangle_filled_RGBA(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	SDL_SetRenderDrawBlendMode(renderer.renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer.renderer, r, g, b, a); 
	SDL_Rect rect = { x, y, w, h };
	SDL_RenderFillRect(renderer.renderer, &rect);
}

int renderer_init(const char *title, unsigned vw, unsigned vh, unsigned scale) {
	gw = vw;
	gh = vh;
	step_scale = scale;
	window_w = gw * step_scale;
	window_h = gh * step_scale;
	windowPos = SDL_WINDOWPOS_CENTERED;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Could not initialize: %s\n", SDL_GetError());
		return -1;
	} 
	
	renderer.sdl_window = SDL_CreateWindow(title, 
		windowPos, 
		windowPos, 
		window_w, 
		window_h, 
		SDL_WINDOW_SHOWN );
	//renderer.screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS);
	//renderer.screen = GPU_InitRenderer(GPU_RENDERER_OPENGL_3, 640, 360, GPU_DEFAULT_INIT_FLAGS);
	
	// if (renderer.screen == NULL) {
	// 	printf("GPU INIT FAILED");
	// 	return -1;
	// }

	Uint32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
	// Uint32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
	renderer.renderer = SDL_CreateRenderer(renderer.sdl_window, -1, flags);

    renderer.clearColor = { 0, 0, 0, 255 };
	SDL_SetRenderDrawColor(renderer.renderer, 0x00, 0x00, 0x00, 0xFF ); 
	
	// Font init
	TTF_Init();

	//Initialize PNG loading 
	int imgFlags = IMG_INIT_PNG; 
	if( !( IMG_Init( imgFlags ) & imgFlags ) ) { 
		printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() ); 
		abort();
	}

    SDL_ShowCursor(SDL_DISABLE);
    
	int displays = SDL_GetNumVideoDisplays();
	printf("detected [ %d ] displays\n", displays);
	if(displays > 1 && PREFERRED_DISPLAY) {
		windowPos = SDL_WINDOWPOS_CENTERED_DISPLAY(1);
	}
	window_set_position(windowPos, windowPos);
    
	SDL_Texture *target = SDL_CreateTexture(renderer.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);
	renderer.renderTarget = target;
	// GPU_Image* renderTargetImage;
	// renderTargetImage = GPU_CreateImage(gw, gh, GPU_FORMAT_RGBA);
	// GPU_LoadTarget(renderTargetImage);
	
    // renderer.renderTargetImage = renderTargetImage;
    // renderer.target = renderTargetImage->target;
	// renderer.sdl_window = SDL_GetWindowFromID(renderer.screen->context->windowID);

	// GPU_SetImageFilter(renderTargetImage, GPU_FILTER_NEAREST);

	// const SDL_Color defaltFontColor = { 255, 255, 255, 255 };
	// defaultFont = Resources::loadFont("default", "data/fonts/pixeltype.ttf", 14);
	// defaultFont->set_color(defaltFontColor);
	
	// camera = GPU_GetDefaultCamera();

    return 1;
}

void renderer_set_clear_color(const SDL_Color &color) {
    renderer.clearColor = color;
}

void renderer_set_color(const SDL_Color &color) {
	SDL_SetRenderDrawColor(renderer.renderer, color.r, color.g, color.b, color.a);
}

void renderer_clear() {
	SDL_SetRenderTarget(renderer.renderer, renderer.renderTarget);
	renderer_set_color(renderer.clearColor);
	SDL_RenderClear(renderer.renderer);
}

void renderer_draw_render_target() {
	SDL_SetRenderTarget(renderer.renderer, NULL);

	// USE TO CREATE BLACK BARS (That can be filled with other things if we want)
	SDL_Rect destination_rect;
	destination_rect.x = (window_w / 2) - (gw * step_scale / 2);
 	destination_rect.y = (window_h / 2) - (gh * step_scale / 2);
  	destination_rect.w = gw * step_scale;
  	destination_rect.h = gh * step_scale;
	SDL_RenderCopy(renderer.renderer, renderer.renderTarget, NULL, &destination_rect);
	
	// USE TO STRETCH TO FILL SCREEN
	// SDL_RenderCopy(renderer.renderer, renderer.renderTarget, NULL, NULL);
}

void renderer_draw_render_target_camera() {
	camera_update();

	SDL_Rect destination_rect;
	destination_rect.x = (window_w / 2) - (gw * step_scale / 2) + camera.offset_x;
 	destination_rect.y = (window_h / 2) - (gh * step_scale / 2) + camera.offset_y;
  	destination_rect.w = gw * step_scale;
  	destination_rect.h = gh * step_scale;

	// if(camera.offset_x != 0) {
	// 	std::string text = std::to_string(destination_rect.x) + " : " + std::to_string(destination_rect.y);
	// 	std::string text2 = std::to_string(camera.offset_x) + " : " + std::to_string(camera.offset_y);
	// 	Engine::logn("%s", text.c_str());
	// 	Engine::logn("%s", text2.c_str());
	// }

	SDL_SetRenderTarget(renderer.renderer, NULL);
	SDL_RenderCopy(renderer.renderer, renderer.renderTarget, NULL, &destination_rect);
}

void renderer_flip() {
    //GPU_Flip(renderer.screen);
	SDL_RenderPresent(renderer.renderer);
}

void renderer_destroy() {
	Resources::cleanup();
	TTF_Quit();
	SDL_DestroyRenderer(renderer.renderer);
	SDL_DestroyWindow(renderer.sdl_window);
}

namespace FrameLog {
	const int max_messages = 20;
    static std::vector<std::string> messages;
	
    void log(const std::string message) {
        if(messages.size() == max_messages) {
            return;
        }
        messages.push_back(message);
    }

    void reset() {
        messages.clear();
    }
	
	void render(int x, int y) {
        int y_start = y;
        for(auto m : messages) {
            draw_text_str(x, y_start, Colors::white, m);
            y_start += 15;
        }
    }
}

void camera_shake(float t) {
	camera.trauma += t;
	camera.trauma = Math::clamp(camera.trauma, 0.0f, 1.0f);
	camera.shake_duration = 0.7f;
}

static const float traumaDropOff = 0.8f; // trauma reduction per 60 frames
static const float maxAngle = 5; // degrees // maxAngle might be something like 5 or 10 degrees
static const float maxOffsetX = 10; // pixels
static const float maxOffsetY = 10; // pixels

void camera_update() {
	if(camera.shake_duration <= 0.0f) {
		camera.trauma = 0.0f;
		camera.shake_duration = 0.0f;
		camera.offset_x = camera.offset_y = 0;
		return;
	}

	// float shake = camera.trauma * camera.trauma; // trauma^2 (or trauma^3)
	// we use simple trauma becuase its so small otherwise
	float shake = camera.trauma; // trauma^2 (or trauma^3)

	float x_r = (float)(RNG::next_i(1) == 0 ? -1.0f : 1.0f);
	float y_r = (float)(RNG::next_i(1) == 0 ? -1.0f : 1.0f);
	
	float offsetX = shake * maxOffsetX * x_r;
	float offsetY = shake * maxOffsetY * y_r;

	camera.offset_x = static_cast<int>(offsetX);
	camera.offset_y = static_cast<int>(offsetY);

	// Special case for shake in top left
	if(camera.offset_x < 0) {
		camera.offset_x *= -1;
	}
	if(camera.offset_y < 0) {
		camera.offset_y *= -1;
	}

	// Engine::logn("trauma: %f   |   offset: %d , %d", camera.trauma, camera.offset_x, camera.offset_y);
	camera.shake_duration -= Time::deltaTime;
	//camera.trauma -= traumaDropOff * Time::deltaTime;
}