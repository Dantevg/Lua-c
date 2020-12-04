#pragma once

#include <lua.h>
#include <lauxlib.h>

#include "tg.h"

/* C library definitions */

typedef struct TermWindow {
	TGContext *tg;
	luaL_Stream *file;
} TermWindow;

/* Lua API definitions */

// Returns the window width
int termWindow_getWidth(lua_State *L);

// Returns the window height
int termWindow_getHeight(lua_State *L);

// Returns the rendering scale
int termWindow_getScale(lua_State *L);

// Sets the rendering scale
int termWindow_setScale(lua_State *L);

// Sets drawing colour
int termWindow_colour(lua_State *L);

// Sets pixel
int termWindow_pixel(lua_State *L);

// Draws a rectangle
int termWindow_rect(lua_State *L);

// Clears the termWindow using the current colour
int termWindow_clear(lua_State *L);

// Draws a character on termWindow
int termWindow_char(lua_State *L);

// Draws a string of characters on termWindow
int termWindow_write(lua_State *L);

// Loads a font
int termWindow_loadFont(lua_State *L);

// Resizes the canvas
// Intended to be used as callback (ignores first argument, event name)
int termWindow_resize(lua_State *L);

int termWindow_present(lua_State *L);

int termWindow_new(lua_State *L);

LUAMOD_API int luaopen_screen_terminal(lua_State *L);