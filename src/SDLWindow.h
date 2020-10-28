#pragma once

#include <SDL2/SDL.h>

#include <lua.h>
#include <lauxlib.h>

#include "font.h"

/* C library definitions */

typedef struct SDLWindow {
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	SDL_Rect rect;
	int scale;
	Font font;
} SDLWindow;

/* Lua API definitions */

// Returns the window width
int SDLWindow_getWidth(lua_State *L);

// Returns the window height
int SDLWindow_getHeight(lua_State *L);

// Returns the rendering scale
int SDLWindow_getScale(lua_State *L);

// Sets the rendering scale
int SDLWindow_setScale(lua_State *L);

// Sets drawing colour
int SDLWindow_colour(lua_State *L);

// Sets pixel
int SDLWindow_pixel(lua_State *L);

// Draws a rectangle
int SDLWindow_rect(lua_State *L);

// Clears the SDLWindow using the current colour
int SDLWindow_clear(lua_State *L);

// Draws a character on SDLWindow
int SDLWindow_char(lua_State *L);

// Draws a string of characters on SDLWindow
int SDLWindow_write(lua_State *L);

// Loads a font
int SDLWindow_loadFont(lua_State *L);

// Resizes the canvas
// Intended to be used as callback (ignores first argument, event name)
int SDLWindow_resize(lua_State *L);

// Presents the buffer on SDLWindow
// Can block if vsync enabled
// FIXME: results in segfault / realloc invalid next size / malloc assertion failed
// when this function doesn't get called often enough (less than 10 times per second)
// and there is mouse movement (?)
int SDLWindow_present(lua_State *L);

int SDLWindow__index(lua_State *L);

int SDLWindow_new(lua_State *L);

static const struct luaL_Reg SDLWindow_f[] = {
	{"getWidth", SDLWindow_getWidth},
	{"getHeight", SDLWindow_getHeight},
	{"getScale", SDLWindow_getScale},
	{"setScale", SDLWindow_setScale},
	{"colour", SDLWindow_colour},
	{"pixel", SDLWindow_pixel},
	{"rect", SDLWindow_rect},
	{"clear", SDLWindow_clear},
	{"char", SDLWindow_char},
	{"write", SDLWindow_write},
	{"loadFont", SDLWindow_loadFont},
	{"resize", SDLWindow_resize},
	{"present", SDLWindow_present},
	{"new", SDLWindow_new},
	{NULL, NULL}
};

int luaopen_SDLWindow(lua_State *L);