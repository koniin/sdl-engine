#ifndef _RENDERER_H
#define _RENDERER_H

#include "engine.h"
#include "SDL_ttf.h"
#include <unordered_map>

// prefered display to show the window
#ifndef PREFERRED_DISPLAY
    #define PREFERRED_DISPLAY 1
#endif

extern unsigned gw;
extern unsigned gh;

typedef struct {
	SDL_Window *sdl_window;
    SDL_Renderer *renderer;
    SDL_Texture *renderTarget;
    SDL_Color default_color;
    SDL_Color clearColor;
} gfx;

extern gfx renderer;

struct Sprite {
    SDL_Texture *image;
    int w;
    int h;
    bool isValid() {
        return image != NULL;
    }
};

struct SpriteFrame {
	int id;
	std::string name;
	SDL_Rect region;
};

struct SpriteSheet {
	std::string sprite_sheet_name;
	std::vector<SpriteFrame> sheet_sprites;
	std::unordered_map<int, int> sprites_by_id;
	std::unordered_map<std::string, int> sprites_by_name;
};

struct Font {
    TTF_Font *font;
    std::string name;
	inline void set_color(const SDL_Color &color) {
		//font->setDefaultColor(color);
	}
};

namespace Resources {
    Sprite *sprite_load(const std::string &name, const std::string &filename);
    Sprite *sprite_get(const std::string &name);
    void sprite_remove(const std::string &name);

    Font *font_load(const std::string name, const std::string filename, int pointSize);
    Font *font_get(const std::string &name);
    void font_remove(const std::string &name);
    
	enum FontStyle {
		NORMAL = 0x00,
		BOLD = 0x01,
		ITALIC = 0x02,
		UNDERLINE = 0x04,
		STRIKETHROUGH = 0x08
	};
    // Styles must be set before drawing text with that font to cache correctly
    void font_set_style(const std::string &name, FontStyle style);
    // Outlines must be set before drawing text with that font to cache correctly
    void font_set_outline(const std::string &name, int outline);

    void sprite_sheet_load(const std::string data, SpriteSheet &s);

    void cleanup();
}

namespace TextCache {
    void clear();
}

namespace Colors {
	const SDL_Color white = { 255, 255, 255, 255 };
	const SDL_Color black = { 0, 0, 0, 255 };
}

void window_set_position(int x, int y);
void window_center();
void window_set_title(const char* title);
void window_set_scale(unsigned s);
void window_toggle_fullscreen(bool useDesktopResolution);
void set_default_font(Font *font);

void draw_sprite(const Sprite *sprite, int x, int y);
void draw_sprite_centered(const Sprite *sprite, int x, int y);
void draw_sprite_region(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y);
void draw_sprite_region_centered(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y);
void draw_sprite_region_centered_rotated(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y, float angle);
void draw_spritesheet_name(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y);
void draw_spritesheet_name_centered(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y);
void draw_spritesheet_name_centered_rotated(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y, const float &angle);
void draw_text(int x, int y, const SDL_Color &color, const char *text);
void draw_text_str(int x, int y, const SDL_Color &color, const std::string text);
void draw_text_font(Font *font, int x, int y, const SDL_Color &color, const char *text);
void draw_text_centered(int x, int y, const SDL_Color &color, const char *text);
void draw_text_centered_str(int x, int y, const SDL_Color &color, std::string text);
void draw_text_font_centered(Font *font, int x, int y, const SDL_Color &color, const char *text);
void draw_tilemap_ortho(const TileMap &t, const SpriteSheet &s, const int x_start, const int y_start);

void draw_g_pixel(Sint16 x, Sint16 y);
void draw_g_pixel_color(Sint16 x, Sint16 y, const SDL_Color &color);
void draw_g_pixel_RGBA(Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void draw_g_horizontal_line(Sint16 x1, Sint16 x2, Sint16 y);
void draw_g_horizontal_line_color(Sint16 x1, Sint16 x2, Sint16 y, SDL_Color &color);
void draw_g_horizontal_line_RGBA(Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void draw_g_vertical_line_color(Sint16 x, Sint16 y1, Sint16 y2, SDL_Color &color);
void draw_g_vertical_line_RGBA(Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void draw_g_circe_color(Sint16 x, Sint16 y, Sint16 rad, SDL_Color &color);
void draw_g_circe_RGBA(Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void draw_g_ellipseRGBA(Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void draw_g_rectangle_filled(int x, int y, int w, int h, const SDL_Color &color);
void draw_g_rectangle_filled_RGBA(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

int renderer_init(const char *title, unsigned vw, unsigned vh, unsigned scale);
void renderer_set_clear_color(const SDL_Color &color);
void renderer_set_color(const SDL_Color &color);
void renderer_clear();
void renderer_draw_render_target();
void renderer_flip();
void renderer_destroy();

namespace FrameLog {
	void log(const std::string message);
	void reset();
    void render(int x, int y);
};

// GPU_Camera *getCamera();
// void renderer_clearCamera();
// void renderer_resetCamera();
// void camera_lookat(int x, int y);
// void camera_move(int x, int y);
#endif