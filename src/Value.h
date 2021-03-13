#pragma once

#include <stdint.h> // For uint8_t etc

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct Value {
	size_t size;
	union {
		uint8_t v8;
		uint16_t v16;
		uint32_t v32;
		uint64_t v64;
	};
} Value;

/* Lua API definitions */

int value_set(lua_State *L);
int value_get(lua_State *L);
int value_of(lua_State *L);
int value_new(lua_State *L);

/* Lua metamethods */

int value__tostring(lua_State *L);
int value__eq(lua_State *L);
int value__lt(lua_State *L);
int value__le(lua_State *L);
int value__add(lua_State *L);
int value__sub(lua_State *L);
int value__mul(lua_State *L);
int value__mod(lua_State *L);
int value__pow(lua_State *L);
int value__div(lua_State *L);
int value__idiv(lua_State *L);
int value__band(lua_State *L);
int value__bor(lua_State *L);
int value__bxor(lua_State *L);
int value__shl(lua_State *L);
int value__shr(lua_State *L);
int value__unm(lua_State *L);
int value__bnot(lua_State *L);

LUAMOD_API int luaopen_value(lua_State *L);
