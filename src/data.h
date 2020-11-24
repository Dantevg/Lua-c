#pragma once

#include <stdint.h> // for uint8_t

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Data {
	size_t size;
	uint8_t data[];
} Data;

Data *data_newdata(lua_State *L, int size);

/* Lua API definitions */

int data_set(lua_State *L);

int data_get(lua_State *L);

int data_of(lua_State *L);

int data_new(lua_State *L);

/* Lua metamethods */

int data_index(lua_State *L);

int data_tostring(lua_State *L);

int data_length(lua_State *L);

LUAMOD_API int luaopen_data(lua_State *L);