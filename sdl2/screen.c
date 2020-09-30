#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.c"

/* C library definitions */

struct Window {
	SDL_Window *window;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	int width;
	int height;
	int scale;
} window;

int get_scale(){
	float scale;
	SDL_RenderGetScale(window.renderer, &scale, NULL);
	return scale;
}

void screen_resize(){
	// Create a new texture
	SDL_Texture *newtexture = SDL_CreateTexture(window.renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window.width, window.height);
	checkSDL(window.texture, "Could not initialize texture: %s\n");
	
	// Set the source and destination rect
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	SDL_QueryTexture(window.texture, NULL, NULL, &rect.w, &rect.h);
	rect.w = (window.width < rect.w) ? window.width : rect.w;
	rect.h = (window.height < rect.h) ? window.height : rect.h;
	
	// Copy over texture data
	SDL_SetRenderTarget(window.renderer, newtexture);
	SDL_RenderCopy(window.renderer, window.texture, &rect, &rect);
	SDL_SetRenderTarget(window.renderer, NULL);
	SDL_DestroyTexture(window.texture);
	window.texture = newtexture;
}

/* Lua API definitions */

// Returns the window width
int screen_getWidth(lua_State *L){
	lua_pushinteger(L, window.width / window.scale);
	
	return 1;
}

// Returns the window height
int screen_getHeight(lua_State *L){
	lua_pushinteger(L, window.height / window.scale);
	
	return 1;
}

// Returns the rendering scale
int screen_getScale(lua_State *L){
	lua_pushinteger(L, window.scale);
	
	return 1;
}

// Sets the rendering scale
int screen_setScale(lua_State *L){
	window.scale = luaL_checkinteger(L, 1);
	
	return 0;
}

// Sets drawing colour
int screen_colour(lua_State *L){
	int r = luaL_checkinteger(L, 1);
	int g = luaL_checkinteger(L, 2);
	int b = luaL_checkinteger(L, 3);
	int a = luaL_checkinteger(L, 4);
	
	SDL_SetRenderDrawColor(window.renderer, r, g, b, a);
	
	return 0;
}

// Sets pixel
int screen_pixel(lua_State *L){
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	
	SDL_RenderDrawPoint(window.renderer, x, y);
	
	return 0;
}

int screen_present(lua_State *L){
	// Display
	SDL_SetRenderTarget(window.renderer, NULL);
	SDL_RenderCopy(window.renderer, window.texture, NULL, NULL);
	SDL_RenderPresent(window.renderer);
	
	// Clear screen
	SDL_SetRenderTarget(window.renderer, window.texture);
	SDL_SetRenderDrawColor(window.renderer, 0, 0, 0, 255);
	SDL_RenderClear(window.renderer);
	SDL_RenderSetScale(window.renderer, window.scale, window.scale);
	
	return 0;
}

int screen_init(lua_State *L){
	window.width = 600;
	window.height = 400;
	window.scale = 2;
	// Create window
	window.window = SDL_CreateWindow("SDL2 Window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window.width, window.height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	checkSDL(window.window, "Could not initialize window: %s\n");
	
	// Create window.renderer
	window.renderer = SDL_CreateRenderer(window.window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	checkSDL(window.renderer, "Could not initialize renderer: %s\n");
	
	// Create texture
	window.texture = SDL_CreateTexture(window.renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window.width, window.height);
	checkSDL(window.texture, "Could not initialize texture: %s\n");
	
	return 0;
}

static const struct luaL_Reg screen[] = {
	{"getWidth", screen_getWidth},
	{"getHeight", screen_getHeight},
	{"getScale", screen_getScale},
	{"setScale", screen_setScale},
	{"colour", screen_colour},
	{"pixel", screen_pixel},
	{"present", screen_present},
	{"init", screen_init},
	{NULL, NULL}
};

int luaopen_screen(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, screen, 0);
	return 1;
}