/***
 * The `Buffer` module provides raw binary data storage and conversion.
 * @module Buffer
 */

#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "Buffer.h"
#include "Value.h"

/* C library definitions */

static const struct luaL_Reg buffer_f[]; // forward-declare for __index

Buffer *buffer_newdata(lua_State *L, int size){
	Buffer *buffer = lua_newuserdata(L, sizeof(Buffer) + sizeof(uint8_t[size]));
	buffer->size = size;
	return buffer;
}

int buffer_within(Buffer *buffer, int position){
	return position >= 0 && (size_t)position < buffer->size;
}

/* Lua API definitions */

/// @type Buffer

int buffer__call(lua_State *L){
	lua_pushcfunction(L, buffer_new); // stack: {buffer_new, (size?), t}
	lua_rotate(L, 1, -1); // stack: {t, buffer_new, (size?)}
	lua_pop(L, 1); // stack: {buffer_new, (size?)}
	lua_rotate(L, 1, -1); // stack: {(size?), buffer_new}
	lua_call(L, lua_gettop(L)-1, 1);
	return 1;
}

/***
 * Set the value starting from the given index.
 * When a string is given, the entire string (or until the end of the `Buffer`,
 * whichever is shortest) will be copied over, so this function can be used
 * for things like `buffer:set(1, "hello")`
 * @function set
 * @tparam number index
 * @tparam number|string value
 */
int buffer_set(lua_State *L){
	Buffer *buffer = luaL_checkudata(L, 1, "Buffer");
	int position = luaL_checkinteger(L, 2) - 1;
	luaL_argcheck(L, buffer_within(buffer, position), 2, "position out of bounds");
	
	switch(lua_type(L, 3)){
		case LUA_TNUMBER:
			buffer->buffer[position] = lua_tointeger(L, 3);
			break;
		case LUA_TSTRING: ; // semicolon here because declaration after case is not allowed in C
			size_t len;
			const char *str = lua_tolstring(L, 3, &len);
			len = (len > buffer->size - position) ? (buffer->size - position) : len;
			memcpy(&buffer->buffer[position], str, len);
			break;
		default:
			luaL_argerror(L, 3, "only number or string elements supported");
	}
	
	return 0;
}

/***
 * Get a value from a given index.
 * @function get
 * @tparam number index
 * @treturn number
 */
int buffer_get(lua_State *L){
	Buffer *buffer = luaL_checkudata(L, 1, "Buffer");
	int position = luaL_checkinteger(L, 2) - 1;
	luaL_argcheck(L, buffer_within(buffer, position), 2, "position out of bounds");
	
	lua_getfield(L, lua_upvalueindex(1), "new"); // Value.new
	lua_pushinteger(L, 1); // stack: {1, Value.new, ...}
	lua_call(L, 1, 1); // stack: {value, ...}
	lua_getfield(L, lua_upvalueindex(1), "set"); // Value.set
	lua_pushvalue(L, -2); // stack: {value, Value.set, value, ...}
	lua_pushinteger(L, buffer->buffer[position]); // stack: {buf[pos], value, Value.set, value, ...}
	lua_call(L, 2, 0); // stack: {value, ...}
	return 1;
}

/// @section end

/***
 * Create a new `Buffer` from one or more values.
 * accepts any number of strings or numbers, or a single table containing them
 * @function of
 * @tparam number|string|table value the value
 * @tparam[opt] number|string ...
 * @treturn Buffer
 */
int buffer_of(lua_State *L){
	Buffer *buffer;
	if(lua_gettop(L) > 1){
		// Multiple values
		buffer = buffer_newdata(L, lua_gettop(L)); // stack: {Buffer, item, item, ...}
		for(size_t i = 0; i < buffer->size; i++){
			buffer->buffer[i] = lua_tointeger(L, i+1);
		}
	}else{
		// Single value, stack: {item}
		switch(lua_type(L, 1)){
			case LUA_TNUMBER:
				buffer = buffer_newdata(L, 1); // stack: {Buffer, item}
				buffer->buffer[0] = lua_tointeger(L, 1);
				break;
			case LUA_TSTRING:
				buffer = buffer_newdata(L, luaL_len(L, 1)); // stack: {Buffer, item}
				strcpy((char*)buffer->buffer, lua_tostring(L, 1));
				break;
			case LUA_TTABLE:
				buffer = buffer_newdata(L, luaL_len(L, 1)); // stack: {Buffer, table}
				for(size_t i = 0; i < buffer->size; i++){
					lua_geti(L, 1, i+1); // stack: {item, Buffer, table}
					buffer->buffer[i] = lua_tointeger(L, 3);
					lua_pop(L, 1); // stack: {Buffer, table}
				}
				break;
			default:
				luaL_error(L, "only numbers, strings or a table containing them supported");
		}
	}
	
	luaL_setmetatable(L, "Buffer");
	return 1;
}

/***
 * Create a new `Buffer` with given size.
 * @function new
 * @tparam number size the size of the buffer
 * @treturn Buffer
 */
int buffer_new(lua_State *L){
	int size = luaL_checkinteger(L, 1);
	Buffer *buffer = lua_newuserdata(L, sizeof(Buffer) + sizeof(uint8_t[size])); // stack: {Buffer, size}
	buffer->size = size;
	
	luaL_setmetatable(L, "Buffer");
	return 1;
}

/* Lua metamethods */

/// @type Buffer

/***
 * __index metamethod, returns the value at the given index, or the function with the given name.
 * When given a number, executes `Buffer:get(k)`. When given a string,
 * acts as if it were a table reference to `Buffer`. That is, returns the function
 * `Buffer[k]` if it exists.
 * @function __index
 * @tparam table table
 * @tparam number|string key
 * @treturn number|function
 */
int buffer__index(lua_State *L){
	// stack: {k, t}
	if(lua_type(L, 2) == LUA_TNUMBER){
		/* Get element at position k */
		lua_pushvalue(L, lua_upvalueindex(2)); // pass Value as upvalue
		lua_pushcclosure(L, buffer_get, 1); // stack: {buffer_get, k, t}
		lua_insert(L, 1); // stack: {k, t, buffer_get}
		lua_call(L, 2, 1); // stack: {v}
		return 1;
	}else{
		/* Get function with name k */
		// This function is put into the metatable with the Buffer table
		// as first upvalue.
		lua_gettable(L, lua_upvalueindex(1));
		return 1;
	}
}

/***
 * __newindex metamethod, sets the value at the given index.
 * @function __newindex
 * @tparam table table
 * @tparam number|string key
 * @param value
 * @treturn number|function
 */
int buffer__newindex(lua_State *L){
	// stack: {v, k, t}
	if(lua_type(L, 2) == LUA_TNUMBER){
		/* Set element at position k */
		lua_pushcfunction(L, buffer_set); // stack: {buffer_set, v, k, t}
		lua_insert(L, 1); // stack: {v, k, t, buffer_set}
		lua_call(L, 3, 0); // stack: {v}
	}
	return 0;
}

/***
 * __tostring metamethod, returns the string representation of the `Buffer`.
 * @function __tostring
 * @treturn string
 */
int buffer__tostring(lua_State *L){
	Buffer *buffer = luaL_checkudata(L, 1, "Buffer");
	lua_pushstring(L, (char*)buffer->buffer);
	return 1;
}

/***
 * __len metamethod, returns the size of the `Buffer`.
 * @function __len
 * @treturn number
 */
int buffer__length(lua_State *L){
	Buffer *buffer = luaL_checkudata(L, 1, "Buffer");
	lua_pushinteger(L, buffer->size);
	return 1;
}

static const struct luaL_Reg buffer_f[] = {
	{"new", buffer_new},
	{"of", buffer_of},
	{"set", buffer_set},
	{"get", buffer_get},
	{"length", buffer__length},
	{NULL, NULL}
};

static const struct luaL_Reg buffer_mt[] = {
	{"__index", buffer__index},
	{"__newindex", buffer__newindex},
	{"__tostring", buffer__tostring},
	{"__len", buffer__length},
	{NULL, NULL}
};

LUAMOD_API int luaopen_Buffer(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, buffer_f, 0);
	
	lua_newtable(L);
	lua_pushcfunction(L, buffer__call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	
	/* Create Buffer metatable */
	if(!luaL_newmetatable(L, "Buffer")){ // stack: {metatable, table, ...}
		luaL_error(L, "couldn't create Buffer metatable");
	}
	
	/* Set metatable */
	lua_pushvalue(L, -2); // duplicate Buffer table for metamethod upvalue
	luaL_requiref(L, "Value", luaopen_Value, 0); // require Value
	luaL_setfuncs(L, buffer_mt, 2); // put buffer_mt functions into metatable,
	// add Buffer and Value table as upvalue
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}
