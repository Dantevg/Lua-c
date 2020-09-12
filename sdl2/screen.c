#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

SDL_Renderer *renderer;

int screen_colour(lua_State *L){
	int r = lua_tointeger(L, -4);
	int g = lua_tointeger(L, -3);
	int b = lua_tointeger(L, -2);
	int a = lua_tointeger(L, -1);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	
	return 0;
}

int screen_pixel(lua_State *L){
	lua_getglobal(L, "scale");
	int scale = lua_tointeger(L, -1);
	int x = lua_tointeger(L, -3);
	int y = lua_tointeger(L, -2);
	
	SDL_RenderSetScale(renderer, scale, scale);
	SDL_RenderDrawPoint(renderer, x, y);
	
	return 0;
}

void screen_init(lua_State *L){
	lua_pushcfunction(L, screen_pixel);
	lua_setglobal(L, "pixel");
	
	lua_pushcfunction(L, screen_colour);
	lua_setglobal(L, "colour");
}