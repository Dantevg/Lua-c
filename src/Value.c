/***
 * The `Value` module provides raw binary single value storage and conversion.
 * @module Value
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "Value.h"

/* C library definitions */

int value_from(lua_State *L, int idx, Value *value){
	switch(lua_type(L, idx)){
		case LUA_TNUMBER: ;
			value->size = sizeof(lua_Integer);
			lua_Integer v = lua_tonumber(L, idx);
			value->v64 = v;
			value->isSigned = (v < 0);
			return 1;
		case LUA_TSTRING:
			value->size = sizeof(uint8_t);
			value->v8 = lua_tostring(L, idx)[0];
			value->isSigned = 0;
			return 1;
		case LUA_TUSERDATA: ;
			Value *ptr = luaL_testudata(L, idx, "Value");
			if(ptr != NULL){
				value->size = ptr->size;
				value->v64 = ptr->v64;
				value->isSigned = ptr->isSigned;
				return 1;
			}
			return 0;
		default:
			return 0;
	}
}

int value_setvalue(lua_State *L, int idx, Value *value){
	switch(lua_type(L, idx)){
		case LUA_TNUMBER: ;
			lua_Integer v = lua_tonumber(L, idx);
			value->v64 = v;
			return 1;
		case LUA_TSTRING:
			value->v8 = lua_tostring(L, idx)[0];
			return 1;
		case LUA_TUSERDATA: ;
			Value *ptr = luaL_testudata(L, idx, "Value");
			if(ptr != NULL){
				value->v64 = ptr->v64;
				return 1;
			}
			return 0;
		default:
			return 0;
	}
}

void value_push(lua_State *L, Value *value){
	switch(value->size){
		case sizeof(uint8_t):  lua_pushinteger(L, value->isSigned ? (int8_t)value->v8 : value->v8);  break;
		case sizeof(uint16_t): lua_pushinteger(L, value->isSigned ? (int16_t)value->v16 : value->v16); break;
		case sizeof(uint32_t): lua_pushinteger(L, value->isSigned ? (int32_t)value->v32 : value->v32); break;
		case sizeof(uint64_t): lua_pushinteger(L, value->isSigned ? (int64_t)value->v64 : value->v64); break;
		default:               lua_pushinteger(L, 0);
	}
}

int value_unop(lua_State *L, int op){
	Value *value = luaL_checkudata(L, 1, "Value");
	lua_pushcfunction(L, value_of);
	value_push(L, value);
	lua_arith(L, op);
	lua_call(L, 1, 1);
	return 1;
}

int value_binop(lua_State *L, int op){
	int correct = 0;
	Value a, b;
	correct += value_from(L, 1, &a);
	correct += value_from(L, 2, &b);
	if(!correct) return 0;
	
	lua_pushcfunction(L, value_of);
	value_push(L, &a);
	value_push(L, &b);
	lua_arith(L, op);
	lua_call(L, 1, 1);
	return 1;
}

int value_cmp(lua_State *L, int op){
	int correct = 0;
	Value a, b;
	correct += value_from(L, 1, &a);
	correct += value_from(L, 2, &b);
	if(!correct) return 0;
	
	value_push(L, &a);
	value_push(L, &b);
	lua_pushboolean(L, lua_compare(L, -2, -1, op));
	return 1;
}

/* Lua API definitions */

/// @type Value

/***
 * Set the value.
 * @function set
 * @tparam number|string|Value value
 */
int value_set(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	
	if(!value_setvalue(L, 2, value)){
		luaL_argerror(L, 2, "Only number, string or Value supported");
	}
	
	return 0;
}

/***
 * Get the numeric value.
 * @function get
 * @treturn number
 */
int value_get(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	value_push(L, value);
	return 1;
}

/// @section end

/***
 * Create a new `Value` from a number, string or other Value.
 * @function of
 * @tparam number|string|Value value
 * @treturn Value
 */
int value_of(lua_State *L){
	Value *value = lua_newuserdata(L, sizeof(Value));
	
	if(!value_from(L, 1, value)){
		luaL_argerror(L, 1, "Only number, string or Value supported");
	}
	
	luaL_setmetatable(L, "Value");
	return 1;
}

/***
 * Create a new empty `Value` of `size` bytes.
 * @function new
 * @tparam[opt=1] number size (1,2,4 or 8)
 * @tparam[optchain=false] boolean signed
 * @treturn Value
 */
int value_new(lua_State *L){
	size_t size = luaL_optinteger(L, 1, sizeof(uint8_t));
	Value *value = lua_newuserdata(L, sizeof(Value));
	value->size = size;
	value->isSigned = lua_toboolean(L, 2);
	if(value->size != 1 && value->size != 2 && value->size != 4 && value->size != 8){
		return luaL_argerror(L, 1, "invalid size (1,2,4 or 8 expected)");
	}
	luaL_setmetatable(L, "Value");
	return 1;
}

/***
 * Get the bits of the value.
 * @function tobits
 * @treturn table
 */
int value_tobits(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	lua_newtable(L);
	int i = 1;
	for(int bit = value->size*8 - 1; bit >= 0; bit--){
		lua_pushboolean(L, (value->v64 >> bit) & 1);
		lua_seti(L, -2, i);
		i++;
	}
	return 1;
}

/***
 * Display the value as its bits.
 * @function tobinary
 * @treturn string
 */
int value_tobinary(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	char binary[value->size*8];
	for(int bit = value->size*8; bit >= 0; bit--){
		binary[value->size*8 - bit - 1] = ((value->v64 >> bit) & 1) ? '1' : '0';
	}
	lua_pushlstring(L, binary, value->size*8);
	return 1;
}

/***
 * Get the size of the value.
 * @function size
 * @treturn number
 */
int value_size(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	lua_pushinteger(L, value->size);
	return 1;
}

/***
 * Get whether the value is signed.
 * @function signed
 * @treturn number
 */
int value_signed(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	lua_pushboolean(L, value->isSigned);
	return 1;
}

/* Lua metamethods */

/// @type Value

int value__call(lua_State *L){
	lua_pushcfunction(L, value_new); // stack: {value_new, (size?), t}
	lua_rotate(L, 1, -1); // stack: {t, value_new, (size?)}
	lua_pop(L, 1); // stack: {value_new, (size?)}
	lua_rotate(L, 1, -1); // stack: {(size?), value_new}
	lua_call(L, lua_gettop(L)-1, 1);
	return 1;
}

/***
 * __tostring metamethod, returns the string representation of the `Value`.
 * @function __tostring
 * @treturn string
 */
int value__tostring(lua_State *L){
	Value *value = luaL_checkudata(L, 1, "Value");
	const char str[] = {(char)value->v8, '\0'};
	lua_pushstring(L, str);
	return 1;
}

int value__eq(lua_State *L){   return value_cmp(L, LUA_OPEQ);     }
int value__lt(lua_State *L){   return value_cmp(L, LUA_OPLT);     }
int value__le(lua_State *L){   return value_cmp(L, LUA_OPLE);     }

int value__add(lua_State *L){  return value_binop(L, LUA_OPADD);  }
int value__sub(lua_State *L){  return value_binop(L, LUA_OPSUB);  }
int value__mul(lua_State *L){  return value_binop(L, LUA_OPMUL);  }
int value__mod(lua_State *L){  return value_binop(L, LUA_OPMOD);  }
int value__pow(lua_State *L){  return value_binop(L, LUA_OPPOW);  }
int value__div(lua_State *L){  return value_binop(L, LUA_OPDIV);  }
int value__idiv(lua_State *L){ return value_binop(L, LUA_OPIDIV); }
int value__band(lua_State *L){ return value_binop(L, LUA_OPBAND); }
int value__bor(lua_State *L){  return value_binop(L, LUA_OPBOR);  }
int value__bxor(lua_State *L){ return value_binop(L, LUA_OPBXOR); }
int value__shl(lua_State *L){  return value_binop(L, LUA_OPSHL);  }
int value__shr(lua_State *L){  return value_binop(L, LUA_OPSHR);  }
int value__unm(lua_State *L){  return value_unop(L, LUA_OPUNM);   }
int value__bnot(lua_State *L){ return value_unop(L, LUA_OPBNOT);  }

static const struct luaL_Reg value_f[] = {
	{"new", value_new},
	{"of", value_of},
	{"set", value_set},
	{"get", value_get},
	{"tobits", value_tobits},
	{"tobinary", value_tobinary},
	{"size", value_size},
	{"signed", value_signed},
	{NULL, NULL}
};

static const struct luaL_Reg value_mt[] = {
	{"__tostring", value__tostring},
	
	{"__eq", value__eq},
	{"__lt", value__lt},
	{"__le", value__le},

	{"__add",  value__add},
	{"__sub",  value__sub},
	{"__mul",  value__mul},
	{"__mod",  value__mod},
	{"__pow",  value__pow},
	{"__div",  value__div},
	{"__idiv", value__idiv},
	{"__band", value__band},
	{"__bor",  value__bor},
	{"__bxor", value__bxor},
	{"__shl",  value__shl},
	{"__shr",  value__shr},
	{"__unm",  value__unm},
	{"__bnot", value__bnot},
	{NULL, NULL}
};

LUAMOD_API int luaopen_Value(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, value_f, 0);
	
	lua_newtable(L);
	lua_pushcfunction(L, value__call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	
	/* Create Value metatable */
	if(!luaL_newmetatable(L, "Value")){ // stack: {metatable, table, ...}
		luaL_error(L, "Couldn't create Value metatable");
	}
	
	/* Set metatable */
	luaL_setfuncs(L, value_mt, 0);
	lua_pushvalue(L, -2); // set Value table as __index
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}
