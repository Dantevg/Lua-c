/***
 * The `data` module provides raw binary data storage and conversion.
 * @module data
 */

#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "data.h"

/* C library definitions */

static const struct luaL_Reg data_f[]; // forward-declare for __index

Data *data_newdata(lua_State *L, int size){
	Data *data = lua_newuserdata(L, sizeof(Data) + sizeof(uint8_t[size]));
	data->size = size;
	return data;
}

int data_within(Data *data, int position){
	return position >= 0 && (size_t)position < data->size;
}

/* Lua API definitions */

/// @type Data

/***
 * Set the value starting from the given index.
 * When a string is given, the entire string (or until the end of the `Data`,
 * whichever is shortest) will be copied over, so this function can be used
 * for things like `data:set(1, "hello")`
 * @function set
 * @tparam number index
 * @tparam number|string value
 */
int data_set(lua_State *L){
	Data *data = luaL_checkudata(L, 1, "Data");
	int position = luaL_checkinteger(L, 2);
	luaL_argcheck(L, data_within(data, position), 2, "position out of bounds");
	
	switch(lua_type(L, 3)){
		case LUA_TNUMBER:
			data->data[position] = lua_tointeger(L, 3);
			break;
		case LUA_TSTRING: ; // semicolon here because declaration after case is not allowed in C
			size_t len;
			const char *str = lua_tolstring(L, 3, &len);
			len = (len > data->size - position) ? (data->size - position) : len;
			memcpy(&data->data[position], str, len);
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
int data_get(lua_State *L){
	Data *data = luaL_checkudata(L, 1, "Data");
	int position = luaL_checkinteger(L, 2);
	luaL_argcheck(L, data_within(data, position), 2, "position out of bounds");
	
	lua_pushinteger(L, data->data[position]);
	return 1;
}

/// @section end

/***
 * Create a new `Data` from one or more values.
 * accepts any number of strings or numbers, or a single table containing them
 * @function of
 * @tparam number|string|table value the value
 * @tparam[opt] number|string ...
 * @treturn Data
 */
int data_of(lua_State *L){
	Data *data;
	if(lua_gettop(L) > 1){
		// Multiple values
		data = data_newdata(L, lua_gettop(L)); // stack: {Data, item, item, ...}
		for(size_t i = 0; i < data->size; i++){
			data->data[i] = lua_tointeger(L, i+1);
		}
	}else{
		// Single value, stack: {item}
		switch(lua_type(L, 1)){
			case LUA_TNUMBER:
				data = data_newdata(L, 1); // stack: {Data, item}
				data->data[0] = lua_tointeger(L, 1);
				break;
			case LUA_TSTRING:
				data = data_newdata(L, luaL_len(L, 1)); // stack: {Data, item}
				strcpy((char*)data->data, lua_tostring(L, 1));
				break;
			case LUA_TTABLE:
				data = data_newdata(L, luaL_len(L, 1)); // stack: {Data, table}
				for(size_t i = 0; i < data->size; i++){
					lua_geti(L, 1, i+1); // stack: {item, Data, table}
					data->data[i] = lua_tointeger(L, 3);
					lua_pop(L, 1); // stack: {Data, table}
				}
				break;
			default:
				luaL_error(L, "only numbers, strings or a table containing them supported");
		}
	}
	
	luaL_setmetatable(L, "Data");
	return 1;
}

/***
 * Create a new `Data` with given size.
 * @function new
 * @tparam number size the size of the data buffer
 * @treturn Data
 */
int data_new(lua_State *L){
	int size = luaL_checkinteger(L, 1);
	Data *data = lua_newuserdata(L, sizeof(Data) + sizeof(uint8_t[size])); // stack: {Data, size}
	data->size = size;
	
	luaL_setmetatable(L, "Data");
	return 1;
}

/* Lua metamethods */

/// @type Data

/***
 * __index metamethod, returns the value at the given index, or the function with the given name.
 * When given a number, executes `Data:get(k)`. When given a string,
 * acts as if it were a table reference to `Data`. That is, returns the function
 * `Data[k]` if it exists.
 * @function __index
 * @tparam table table
 * @tparam number|string key
 * @treturn number|function
 */
int data_index(lua_State *L){
	// stack: {k, t}
	if(lua_type(L, 2) == LUA_TNUMBER){
		/* Get element at position k */
		lua_pushcfunction(L, data_get); // stack: {data_get, k, t}
		lua_insert(L, 1); // stack: {k, t, data_get}
		lua_call(L, 2, 1); // stack: {v}
		return 1;
	}else{
		/* Get function with name k */
		// This function is put into the metatable with the Data table
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
int data_newindex(lua_State *L){
	// stack: {v, k, t}
	if(lua_type(L, 2) == LUA_TNUMBER){
		/* Set element at position k */
		lua_pushcfunction(L, data_set); // stack: {data_set, v, k, t}
		lua_insert(L, 1); // stack: {v, k, t, data_set}
		lua_call(L, 3, 0); // stack: {v}
	}
	return 0;
}

/***
 * __tostring metamethod, returns the string representation of the `Data`.
 * @function __tostring
 * @treturn string
 */
int data_tostring(lua_State *L){
	Data *data = luaL_checkudata(L, 1, "Data");
	lua_pushstring(L, (char*)data->data);
	return 1;
}

/***
 * __len metamethod, returns the size of the `Data`.
 * @function __len
 * @treturn number
 */
int data_length(lua_State *L){
	Data *data = luaL_checkudata(L, 1, "Data");
	lua_pushinteger(L, data->size);
	return 1;
}

static const struct luaL_Reg data_f[] = {
	{"new", data_new},
	{"of", data_of},
	{"set", data_set},
	{"get", data_get},
	{"tostring", data_tostring},
	{"length", data_length},
	{NULL, NULL}
};

static const struct luaL_Reg data_mt[] = {
	{"__index", data_index},
	{"__newindex", data_newindex},
	{"__tostring", data_tostring},
	{"__len", data_length},
	{NULL, NULL}
};

LUAMOD_API int luaopen_data(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, data_f, 0);
	
	/* Create Data metatable */
	if(!luaL_newmetatable(L, "Data")){ // stack: {metatable, table, ...}
		luaL_error(L, "couldn't create Data metatable");
	}
	
	/* Set metatable */
	lua_pushvalue(L, -2); // duplicate Data table for metamethod upvalue
	luaL_setfuncs(L, data_mt, 1); // put data_mt functions into metatable,
	// add Data table as upvalue
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}
