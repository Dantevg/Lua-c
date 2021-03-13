#pragma once

#include <stdint.h> // for uint8_t etc

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Buffer {
	size_t size;
	uint8_t buffer[];
} Buffer;

Buffer *buffer_newbuffer(lua_State *L, int size);

/* Lua API definitions */

int buffer_set(lua_State *L);
int buffer_get(lua_State *L);
int buffer_of(lua_State *L);
int buffer_new(lua_State *L);

/* Lua metamethods */

int buffer__index(lua_State *L);
int buffer__tostring(lua_State *L);
int buffer__length(lua_State *L);

LUAMOD_API int luaopen_Buffer(lua_State *L);
