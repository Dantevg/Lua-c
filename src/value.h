#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

int value_set(lua_State *L);

int value_get(lua_State *L);

int value_of(lua_State *L);

int value_new(lua_State *L);

/* Lua metamethods */

int value_tostring(lua_State *L);
int value_eq(lua_State *L);
int value_lt(lua_State *L);
int value_le(lua_State *L);
int value_add(lua_State *L);
int value_sub(lua_State *L);
int value_mod(lua_State *L);

LUAMOD_API int luaopen_value(lua_State *L);
