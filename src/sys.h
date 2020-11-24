#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

// Change directory
int sys_chdir(lua_State *L);

LUAMOD_API int luaopen_sys(lua_State *L);