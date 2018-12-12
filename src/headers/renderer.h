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

struct Camera {
	float shake_duration = 0.0f;
	float trauma = 0.0f;
	float x = 0;
	float y = 0;
	float offset_x = 0;
	float offset_y = 0;
    float follow_x = 0;
    float follow_y = 0;
    float speed = 0.2f;

    // initialize with really large values
    // used to clamp the camera at edges of level
    float x_min = -900000.0f;
    float x_max = 900000.0f;
    float y_min = -900000.0f;
    float y_max = 900000.0f;
};

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
    Sprite *sprite_load_white(const std::string &name, const std::string &filename);
    Sprite *sprite_get(const std::string &name);
    SDL_Rect &sprite_get_from_sheet(const size_t &sprite_sheet_index, const std::string &name);
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

    void sprite_sheet_load(const std::string &name, const std::string &file);
    void sprite_sheet_copy_as_white(const std::string &name, const std::string &copy_from);
    size_t sprite_sheet_index(const std::string &name);
    const SpriteSheet &sprite_sheet_get(const std::string &name);
    std::vector<SpriteSheet> &get_sprite_sheets();

    void cleanup();
}

namespace TextCache {
    void clear();
}

namespace Colors {
	const SDL_Color white = { 255, 255, 255, 255 };
	const SDL_Color black = { 0, 0, 0, 255 };

    inline SDL_Color make(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        return { r, g, b, a };
    }
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
void draw_sprite_region_centered_ex(const Sprite *sprite, const SDL_Rect *src_rect, int x, int y, int w, int h, float angle);
void draw_spritesheet_name(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y);
void draw_spritesheet_name_centered(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y);
void draw_spritesheet_name_centered_rotated(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y, const float &angle);
void draw_spritesheet_name_centered_ex(const SpriteSheet &s, const std::string &sprite, const int &x, const int &y, const int &w, const int &h, const float &angle);
void draw_text(int x, int y, const SDL_Color &color, const char *text);
void draw_text_str(int x, int y, const SDL_Color &color, const std::string text);
void draw_text_font(Font *font, int x, int y, const SDL_Color &color, const char *text);
void draw_text_centered(int x, int y, const SDL_Color &color, const char *text);
void draw_text_centered_str(int x, int y, const SDL_Color &color, std::string text);
void draw_text_right_str(int x, int y, const SDL_Color &color, std::string text);
void draw_text_font_centered(Font *font, int x, int y, const SDL_Color &color, const char *text);
void draw_text_font_right_aligned(Font *font, int x, int y, const SDL_Color &color, const char *text);
void draw_tilemap_ortho(const TileMap &t, const SpriteSheet &s, const int x_start, const int y_start);

void draw_g_pixel(int x, int y);
void draw_g_pixel_color(int x, int y, const SDL_Color &color);
void draw_g_pixel_RGBA(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_line(int x1, int y1, int x2, int y2);
void draw_g_line_RGBA(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_horizontal_line(int x1, int x2, int y);
void draw_g_horizontal_line_color(int x1, int x2, int y, SDL_Color &color);
void draw_g_horizontal_line_RGBA(int x1, int x2, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_vertical_line_color(int x, int y1, int y2, SDL_Color &color);
void draw_g_vertical_line_RGBA(int x, int y1, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_circle_color(int x, int y, int rad, SDL_Color &color);
void draw_g_circle_RGBA(int x, int y, int rad, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_circle_filled_color(int x, int y, int rad, SDL_Color &color);
void draw_g_circle_filled_RGBA(int x, int y, int rad, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_ellipseRGBA(int x, int y, int rx, int ry, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_rectangle(int x, int y, int w, int h, const SDL_Color &color);
void draw_g_rectangle_RGBA(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_g_rectangle_filled(int x, int y, int w, int h, const SDL_Color &color);
void draw_g_rectangle_filled_RGBA(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

int renderer_init(const char *title, unsigned vw, unsigned vh, unsigned scale);
void renderer_set_clear_color(const SDL_Color &color);
void renderer_set_color(const SDL_Color &color);
void renderer_clear();
void renderer_draw_render_target();
void renderer_draw_render_target_camera();
void renderer_flip();
void renderer_destroy();

const Camera &get_camera();
void camera_follow(Vector2 position);
void camera_lookat(Vector2 position);
void camera_displace(Vector2 displacement);
/*! Trauma should be between 0 and 1. */
void camera_shake(float t);
void camera_update();
void camera_set_clamp_area(float x_min, float x_max, float y_min, float y_max);
void camera_reset_clamp_area();


#endif