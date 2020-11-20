#pragma once

#include <SDL2/SDL.h>

#include <lua.h>
#include <lauxlib.h>

#include "../font.h"

/* C library definitions */

typedef struct SDLImage {
	SDL_Surface *surface;
	SDL_Renderer *renderer;
	SDL_Rect rect;
	int scale;
	Font font;
} SDLImage;

/* Lua API definitions */

// Returns the image width
int SDLImage_getWidth(lua_State *L);

// Returns the image height
int SDLImage_getHeight(lua_State *L);

// Returns the rendering scale
int SDLImage_getScale(lua_State *L);

// Sets the rendering scale
int SDLImage_setScale(lua_State *L);

// Sets drawing colour
int SDLImage_colour(lua_State *L);

// Sets pixel
int SDLImage_pixel(lua_State *L);

// Draws a rectangle
int SDLImage_rect(lua_State *L);

// Clears the image using the current colour
int SDLImage_clear(lua_State *L);

// Draws a character on the image
int SDLImage_char(lua_State *L);

// Draws a string of characters on the image
int SDLImage_write(lua_State *L);

// Gets the pixel colour on the given coordinates
int SDLImage_getPixel(lua_State *L);

// Loads a font
int SDLImage_loadFont(lua_State *L);

// Resizes the image
// Intended to be used as callback for screen interface compatibility
// (ignores first argument, event name)
int SDLImage_resize(lua_State *L);

// For screen interface compatibility
int SDLImage_present(lua_State *L);

// Saves the image to a file
int SDLImage_save(lua_State *L);

int SDLImage_new(lua_State *L);

LUAMOD_API int luaopen_image_SDLImage(lua_State *L);