/***
 * The `sys` module gives access to system information.
 * @module sys
 */

#include <unistd.h> // for chdir, strerror

#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

/***
 * Change directory to `path`.
 * @function chdir
 * @tparam string path
 * @treturn boolean success
 * @treturn string|nil error
 */
int sys_chdir(lua_State *L){
	const char *path = luaL_checkstring(L, 1);
	int status = chdir(path);
	if(status == 0){
		lua_pushboolean(L, 1);
		return 1;
	}else{
		lua_pushboolean(L, 0);
		lua_pushstring(L, strerror(status));
		return 2;
	}
}

/*** 
 * The number of CPU cores
 * @tfield number cores
 */

/***
 * The amount of RAM, in MB
 * @tfield number ram
 */

/***
 * The name of the OS
 * @tfield string os
 */

static const struct luaL_Reg sys_f[] = {
	{"chdir", sys_chdir},
	{NULL, NULL}
};

LUAMOD_API int luaopen_sys(lua_State *L){
	lua_newtable(L); // stack: {table}
	luaL_setfuncs(L, sys_f, 0);
	
	lua_pushinteger(L, SDL_GetCPUCount());
	lua_setfield(L, -2, "cores");
	
	lua_pushinteger(L, SDL_GetSystemRAM());
	lua_setfield(L, -2, "ram");
	
	lua_pushstring(L, SDL_GetPlatform());
	lua_setfield(L, -2, "os");
	
	return 1;
}