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

Buffer *buffer_newbuffer(lua_State *L, lua_Integer size);
int buffer_within(Buffer *buffer, lua_Integer position);
void buffer_set_with_size(Buffer *buffer, lua_Integer index, lua_Integer value, size_t size, int littleEndian);
lua_Integer buffer_get_with_size(Buffer *buffer, lua_Integer index, size_t size, int littleEndian, int isSigned);

/* Lua API definitions */

int buffer_of(lua_State *L);
int buffer_new(lua_State *L);
int buffer__call(lua_State *L);
int buffer_set(lua_State *L);
int buffer_get(lua_State *L);
int buffer_stream(lua_State *L);
int buffer_view(lua_State *L);

int buffer_setUint8(lua_State *L);
int buffer_setInt8(lua_State *L);
int buffer_setUint16(lua_State *L);
int buffer_setInt16(lua_State *L);
int buffer_setUint32(lua_State *L);
int buffer_setInt32(lua_State *L);
int buffer_setUint64(lua_State *L);
int buffer_setInt64(lua_State *L);

int buffer_getUint8(lua_State *L);
int buffer_getInt8(lua_State *L);
int buffer_getUint16(lua_State *L);
int buffer_getInt16(lua_State *L);
int buffer_getUint32(lua_State *L);
int buffer_getInt32(lua_State *L);
int buffer_getUint64(lua_State *L);
int buffer_getInt64(lua_State *L);

/* Lua metamethods */

int buffer__index(lua_State *L);
int buffer__newindex(lua_State *L);
int buffer__tostring(lua_State *L);
int buffer__length(lua_State *L);

LUAMOD_API int luaopen_Buffer(lua_State *L);
