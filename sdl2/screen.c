#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

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

void screen_createWindow(){
	window.width = 600;
	window.height = 400;
	window.scale = 2;
	// Create window
	window.window = SDL_CreateWindow("Test window",
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
	window.scale = lua_tointeger(L, -1);
	
	return 0;
}

// Sets drawing colour
int screen_colour(lua_State *L){
	int r = lua_tointeger(L, -4);
	int g = lua_tointeger(L, -3);
	int b = lua_tointeger(L, -2);
	int a = lua_tointeger(L, -1);
	SDL_SetRenderDrawColor(window.renderer, r, g, b, a);
	
	return 0;
}

// Sets pixel
int screen_pixel(lua_State *L){
	int x = lua_tointeger(L, -2);
	int y = lua_tointeger(L, -1);
	
	SDL_RenderDrawPoint(window.renderer, x, y);
	
	return 0;
}

void screen_init(lua_State *L){
	lua_newtable(L); // stack: {table, ...}
	lua_pushvalue(L, -1); // stack: {table, table, ...}
	lua_setglobal(L, "screen"); // stack: {table, ...}
	
	lua_pushcfunction(L, screen_getWidth); // stack: {screen_getWidth, table, ...}
	lua_setfield(L, -2, "getWidth"); // stack: {table, ...}
	
	lua_pushcfunction(L, screen_getHeight); // stack: {screen_getHeight, table, ...}
	lua_setfield(L, -2, "getHeight"); // stack: {table, ...}
	
	lua_pushcfunction(L, screen_getScale); // stack: {screen_getScale, table, ...}
	lua_setfield(L, -2, "getScale"); // stack: {table, ...}
	
	lua_pushcfunction(L, screen_setScale); // stack: {screen_setScale, table, ...}
	lua_setfield(L, -2, "setScale"); // stack: {table, ...}
	
	lua_pushcfunction(L, screen_pixel); // stack: {screen_pixel, table, ...}
	lua_setfield(L, -2, "pixel"); // stack: {table, ...}
	
	lua_pushcfunction(L, screen_colour); // stack: {screen_colour, table, ...}
	lua_setfield(L, -2, "colour"); // stack: {table, ...}
	lua_pop(L, 1); // stack: {...}

	screen_createWindow();
}