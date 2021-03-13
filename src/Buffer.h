#pragma once

#include <stdint.h> // for uint8_t etc

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Buffer {
	size_t size;     // size of the buffer
	uint8_t *buffer; // pointer to the data, can point to its own data or somewhere else
	uint8_t data[];  // the actual data
} Buffer;

Buffer *buffer_newbuffer(lua_State *L, int size);

/* Lua API definitions */

int buffer_of(lua_State *L);
int buffer_new(lua_State *L);
int buffer_set(lua_State *L);
int buffer_get(lua_State *L);
int buffer_stream(lua_State *L);
int buffer_view(lua_State *L);

/* Lua metamethods */

int buffer__index(lua_State *L);
int buffer__tostring(lua_State *L);
int buffer__length(lua_State *L);

LUAMOD_API int luaopen_Buffer(lua_State *L);
