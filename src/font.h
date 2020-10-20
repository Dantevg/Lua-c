#pragma once

#include <SDL2/SDL.h>

#include <lua.h>

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

Font font_load(lua_State *L, SDL_Renderer *renderer);

int font_char(Font *font, SDL_Renderer *renderer, SDL_Rect *dest, char c);