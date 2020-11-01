#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

int luaopen_sys(lua_State *L){
	lua_newtable(L); // stack: {table}
	
	lua_pushinteger(L, SDL_GetCPUCount());
	lua_setfield(L, -2, "cores");
	
	lua_pushinteger(L, SDL_GetSystemRAM());
	lua_setfield(L, -2, "ram");
	
	lua_pushstring(L, SDL_GetPlatform());
	lua_setfield(L, -2, "os");
	
	return 1;
}