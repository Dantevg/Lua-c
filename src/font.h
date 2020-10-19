#pragma once

#include <SDL2/SDL.h>
// #include <lua.h>

typedef struct Char {
	char c;
	SDL_Rect rect;
	int ox;
	int oy;
	int advance;
} Char;

typedef struct Font {
	SDL_Texture *image;
	int height;
	Char chars[256];
} Font;

void font_transfer_int_field(lua_State *L, int i, const char *name, int *dst);

Font font_load(lua_State *L, SDL_Renderer *renderer);

int font_char(SDL_Renderer *renderer, Font *font, SDL_Rect *dest, char c);