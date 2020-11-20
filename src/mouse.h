#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

// Returns the mouse position
int mouse_pos(lua_State *L);

// Returns whether the given button is down
int mouse_down(lua_State *L);

LUAMOD_API int luaopen_mouse(lua_State *L);