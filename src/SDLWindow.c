#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "SDLWindow.h"
#include "font.h"

/* C library definitions */

/* Lua API definitions */

// Returns the window width
int SDLWindow_getWidth(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	lua_pushinteger(L, window->rect.w / window->scale);
	
	return 1;
}

// Returns the window height
int SDLWindow_getHeight(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	lua_pushinteger(L, window->rect.h / window->scale);
	
	return 1;
}

// Returns the rendering scale
int SDLWindow_getScale(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	lua_pushinteger(L, window->scale);
	
	return 1;
}

// Sets the rendering scale
int SDLWindow_setScale(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	window->scale = luaL_checkinteger(L, 2);
	
	return 0;
}

// Sets drawing colour
int SDLWindow_colour(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	int r = luaL_checkinteger(L, 2);
	int g = luaL_optinteger(L, 3, r);
	int b = luaL_optinteger(L, 4, r);
	int a = luaL_optinteger(L, 5, 255);
	
	SDL_SetRenderDrawColor(window->renderer, r, g, b, a);
	
	return 0;
}

// Sets pixel
int SDLWindow_pixel(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	SDL_RenderDrawPoint(window->renderer, x, y);
	
	return 0;
}

// Draws a rectangle
int SDLWindow_rect(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	
	SDL_Rect rect;
	rect.x = luaL_checkinteger(L, 2);
	rect.y = luaL_checkinteger(L, 3);
	rect.w = luaL_checkinteger(L, 4);
	rect.h = luaL_checkinteger(L, 5);
	int fill = lua_toboolean(L, 6);
	
	if(fill){
		SDL_RenderFillRect(window->renderer, &rect);
	}else{
		SDL_RenderDrawRect(window->renderer, &rect);
	}
	
	return 0;
}

// Clears the SDLWindow canvas using the current colour
int SDLWindow_clear(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	SDL_RenderClear(window->renderer);
	
	return 0;
}

int SDLWindow_char(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	const char *str = luaL_checkstring(L, 2);
	SDL_Rect rect;
	rect.x = luaL_checkinteger(L, 3);
	rect.y = luaL_checkinteger(L, 4);
	font_char(&window->font, window->renderer, &rect, str[0]);
	
	return 0;
}

int SDLWindow_write(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	const char *str = luaL_checkstring(L, 2);
	
	/* Get string length */
	lua_len(L, 2);
	int n = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	/* Set initial position rect */
	int x = luaL_checkinteger(L, 3);
	int y = luaL_checkinteger(L, 4);
	SDL_Rect rect;
	
	/* Draw characters */
	for(int i = 0; i < n; i++){
		rect.x = x;
		rect.y = y;
		x += font_char(&window->font, window->renderer, &rect, str[i]);
	}
	return 0;
}

int SDLWindow_loadFont(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow"); // stack: {filename, SDLWindow}
	lua_replace(L, 1); // stack: {filename}
	window->font = font_load(L, window->renderer);
	return 0;
}

// Resizes the SDLWindow canvas
int SDLWindow_resize(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	
	window->rect.w = luaL_checkinteger(L, 2);
	window->rect.h = luaL_checkinteger(L, 3);
	
	/* Create a new texture */
	SDL_Texture *newtexture = SDL_CreateTexture(window->renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window->rect.w, window->rect.h);
	checkSDL(window->texture, "could not initialize texture: %s\n");
	
	/* Set the source and destination rect */
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	SDL_QueryTexture(window->texture, NULL, NULL, &rect.w, &rect.h);
	rect.w = (window->rect.w < rect.w) ? window->rect.w : rect.w;
	rect.h = (window->rect.h < rect.h) ? window->rect.h : rect.h;
	
	/* Copy over texture data */
	SDL_SetRenderTarget(window->renderer, newtexture);
	SDL_RenderCopy(window->renderer, window->texture, &rect, &rect);
	SDL_SetRenderTarget(window->renderer, NULL);
	SDL_DestroyTexture(window->texture);
	window->texture = newtexture;
	
	return 0;
}

// Presents the buffer on SDLWindow
// Can block if vsync enabled
int SDLWindow_present(lua_State *L){
	SDLWindow *window = luaL_checkudata(L, 1, "SDLWindow");
	
	/* Display */
	SDL_SetRenderTarget(window->renderer, NULL);
	SDL_RenderCopy(window->renderer, window->texture, &window->rect, &window->rect);
	SDL_RenderSetScale(window->renderer, window->scale, window->scale);
	SDL_RenderPresent(window->renderer);
	
	/* Reset render target */
	SDL_SetRenderTarget(window->renderer, window->texture);
	SDL_RenderSetScale(window->renderer, 1, 1);
	
	return 0;
}

int SDLWindow_new(lua_State *L){
	/* Create SDLWindow */
	SDLWindow *window = lua_newuserdata(L, sizeof(SDLWindow)); // stack: {SDLWindow}
	window->rect.x = 0;
	window->rect.y = 0;
	window->rect.w = 600;
	window->rect.h = 400;
	window->scale = 2;
	
	/* Create window */
	window->window = SDL_CreateWindow("SDL2 Window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window->rect.w, window->rect.h,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	checkSDL(window->window, "could not initialize window: %s\n");
	
	/* Create renderer */
	window->renderer = SDL_CreateRenderer(window->window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	checkSDL(window->renderer, "could not initialize renderer: %s\n");
	
	/* Create texture */
	window->texture = SDL_CreateTexture(window->renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window->rect.w, window->rect.h);
	checkSDL(window->texture, "could not initialize texture: %s\n");
	
	/* Set default colour to white */
	SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
	
	/* Return window, SDLWindow userdata is still on stack */
	luaL_setmetatable(L, "SDLWindow"); // Set the SDLWindow metatable to the userdata
	return 1;
}

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

LUAMOD_API int luaopen_SDLWindow(lua_State *L){
	lua_newtable(L); // stack: {table, ...}
	luaL_setfuncs(L, SDLWindow_f, 0);
	
	/* Create SDLWindow metatable */
	if(!luaL_newmetatable(L, "SDLWindow")){ // stack: {metatable, table, ...}
		luaL_error(L, "couldn't create SDLWindow metatable");
	}
	
	/* Set __index to SDLWindow, for OO */
	lua_pushvalue(L, -2); // stack: {table, metatable, table, ...}
	lua_setfield(L, -2, "__index"); // stack: {metatable, table, ...}
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}