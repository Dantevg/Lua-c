#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

/* Lua API definitions */

// Returns whether a key is down
int kb_down(lua_State *L);

// Returns whether a physical key is down
int kb_scancodeDown(lua_State *L);

static const struct luaL_Reg kb_f[] = {
	{"down", kb_down},
	{"scancodeDown", kb_scancodeDown},
	{NULL, NULL}
};

int luaopen_kb(lua_State *L);