#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "mouse.h"

/* C library definitions */

/* Lua API definitions */

// Returns the mouse position
int mouse_pos(lua_State *L){
	int x, y;
	SDL_GetMouseState(&x, &y);
	
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	
	return 2;
}

// Returns whether the given button is down
int mouse_down(lua_State *L){
	int btn = luaL_checkinteger(L, 1);
	uint32_t btns = SDL_GetMouseState(NULL, NULL);
	
	lua_pushboolean(L, btns & SDL_BUTTON(btn));
	return 1;
}

LUAMOD_API int luaopen_mouse(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, mouse_f, 0);
	return 1;
}