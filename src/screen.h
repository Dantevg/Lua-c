#pragma once

#include <SDL2/SDL.h>

#include <lua.h>
#include <lauxlib.h>

#include "font.h"

/* C library definitions */

struct Window {
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	SDL_Rect rect;
	int scale;
	Font font;
} window;

int get_scale();

/* Lua API definitions */

// Returns the window width
int screen_getWidth(lua_State *L);

// Returns the window height
int screen_getHeight(lua_State *L);

// Returns the rendering scale
int screen_getScale(lua_State *L);

// Sets the rendering scale
int screen_setScale(lua_State *L);

// Sets drawing colour
int screen_colour(lua_State *L);

// Sets pixel
int screen_pixel(lua_State *L);

// Clears the screen using the current colour
int screen_clear(lua_State *L);

// Draws a character on screen
int screen_char(lua_State *L);

// Draws a string of characters on screen
int screen_write(lua_State *L);

// Loads a font
int screen_loadFont(lua_State *L);

// Resizes the canvas
// Intended to be used as callback (ignores first argument, event name)
int screen_resize(lua_State *L);

// Presents the buffer on screen
// Can block if vsync enabled
// FIXME: results in segfault / realloc invalid next size / malloc assertion failed
// when this function doesn't get called often enough (less than 10 times per second)
// and there is mouse movement (?)
int screen_present(lua_State *L);

static const struct luaL_Reg screen[] = {
	{"getWidth", screen_getWidth},
	{"getHeight", screen_getHeight},
	{"getScale", screen_getScale},
	{"setScale", screen_setScale},
	{"colour", screen_colour},
	{"pixel", screen_pixel},
	{"clear", screen_clear},
	{"char", screen_char},
	{"write", screen_write},
	{"loadFont", screen_loadFont},
	{"resize", screen_resize},
	{"present", screen_present},
	{NULL, NULL}
};

int luaopen_screen(lua_State *L);