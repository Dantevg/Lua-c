/***
 * The `value` module provides raw binary single value storage and conversion.
 * @module value
 */

#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "value.h"

/* C library definitions */

int value_from(lua_State *L, int idx, uint8_t *data){
	switch(lua_type(L, idx)){
		case LUA_TNUMBER:
			*data = lua_tonumber(L, idx);
			return 1;
		case LUA_TSTRING:
			*data = lua_tostring(L, idx)[0];
			return 1;
		case LUA_TUSERDATA: ;
			uint8_t *ptr = luaL_testudata(L, idx, "Value");
			if(ptr != NULL){
				*data = *ptr;
				return 1;
			}
	}
	return 0;
}

int value_unop(lua_State *L, int op){
	uint8_t a;
	if(!value_from(L, 1, &a)) return 0;
	lua_pushcfunction(L, value_of);
	lua_pushinteger(L, a);
	lua_arith(L, op);
	lua_call(L, 1, 1);
	return 1;
}

int value_binop(lua_State *L, int op){
	int correct = 0;
	uint8_t a, b;
	correct += value_from(L, 1, &a);
	correct += value_from(L, 2, &b);
	
	if(!correct) return 0;
	lua_pushcfunction(L, value_of);
	lua_pushinteger(L, a);
	lua_pushinteger(L, b);
	lua_arith(L, op);
	lua_call(L, 1, 1);
	return 1;
}

int value_cmp(lua_State *L, int op){
	int correct = 0;
	uint8_t a, b;
	correct += value_from(L, 1, &a);
	correct += value_from(L, 2, &b);
	
	if(!correct) return 0;
	lua_pushinteger(L, a);
	lua_pushinteger(L, b);
	lua_pushboolean(L, lua_compare(L, -2, -1, op));
	return 1;
}

/* Lua API definitions */

/// @type Value

/***
 * Set the value.
 * @function set
 * @tparam number|string value
 */
int value_set(lua_State *L){
	uint8_t *data = luaL_checkudata(L, 1, "Value");
	
	switch(lua_type(L, 3)){
		case LUA_TNUMBER:
			*data = lua_tointeger(L, 2);
			break;
		case LUA_TSTRING: ; // semicolon here because declaration after case is not allowed in C
			*data = lua_tostring(L, 2)[0];
			break;
		default:
			luaL_argerror(L, 2, "Only number or string supported");
	}
	
	return 0;
}

/***
 * Get the numeric value.
 * @function get
 * @treturn number
 */
int value_get(lua_State *L){
	uint8_t *data = luaL_checkudata(L, 1, "Value");
	lua_pushinteger(L, *data);
	return 1;
}

/// @section end

/***
 * Create a new `Value` from a number or string.
 * @function of
 * @tparam number|string value the value
 * @treturn Value
 */
int value_of(lua_State *L){
	uint8_t *data = lua_newuserdata(L, sizeof(uint8_t));
	switch(lua_type(L, 1)){
		case LUA_TNUMBER:
			*data = lua_tointeger(L, 1);
			break;
		case LUA_TSTRING:
			*data = lua_tostring(L, 1)[0];
			break;
		default:
			luaL_error(L, "Only number or string supported");
	}
	
	luaL_setmetatable(L, "Value");
	return 1;
}

/***
 * Create a new empty `Value`.
 * @function new
 * @treturn Value
 */
int value_new(lua_State *L){
	lua_newuserdata(L, sizeof(uint8_t));
	luaL_setmetatable(L, "Value");
	return 1;
}

/* Lua metamethods */

/// @type Value

/***
 * __tostring metamethod, returns the string representation of the `Value`.
 * @function __tostring
 * @treturn string
 */
int value_tostring(lua_State *L){
	uint8_t *data = luaL_checkudata(L, 1, "Value");
	const char str[] = {(char)*data, '\0'};
	lua_pushstring(L, str);
	return 1;
}

int value_eq(lua_State *L){   return value_cmp(L, LUA_OPEQ);     }
int value_lt(lua_State *L){   return value_cmp(L, LUA_OPLT);     }
int value_le(lua_State *L){   return value_cmp(L, LUA_OPLE);     }

int value_add(lua_State *L){  return value_binop(L, LUA_OPADD);  }
int value_sub(lua_State *L){  return value_binop(L, LUA_OPSUB);  }
int value_mul(lua_State *L){  return value_binop(L, LUA_OPMUL);  }
int value_mod(lua_State *L){  return value_binop(L, LUA_OPMOD);  }
int value_pow(lua_State *L){  return value_binop(L, LUA_OPPOW);  }
int value_div(lua_State *L){  return value_binop(L, LUA_OPDIV);  }
int value_idiv(lua_State *L){ return value_binop(L, LUA_OPIDIV); }
int value_band(lua_State *L){ return value_binop(L, LUA_OPBAND); }
int value_bor(lua_State *L){  return value_binop(L, LUA_OPBOR);  }
int value_bxor(lua_State *L){ return value_binop(L, LUA_OPBXOR); }
int value_shl(lua_State *L){  return value_binop(L, LUA_OPSHL);  }
int value_shr(lua_State *L){  return value_binop(L, LUA_OPSHR);  }
int value_unm(lua_State *L){  return value_unop(L, LUA_OPUNM);   }
int value_bnot(lua_State *L){ return value_unop(L, LUA_OPBNOT);  }

static const struct luaL_Reg value_f[] = {
	{"new", value_new},
	{"of", value_of},
	{"set", value_set},
	{"get", value_get},
	{"tostring", value_tostring},
	{NULL, NULL}
};

static const struct luaL_Reg value_mt[] = {
	{"__tostring", value_tostring},
	
	{"__eq", value_eq},
	{"__lt", value_lt},
	{"__le", value_le},

	{"__add", value_add},
	{"__sub", value_sub},
	{"__mul", value_mul},
	{"__mod", value_mod},
	{"__pow", value_pow},
	{"__div", value_div},
	{"__idiv", value_idiv},
	{"__band", value_band},
	{"__bor", value_bor},
	{"__bxor", value_bxor},
	{"__shl", value_shl},
	{"__shr", value_shr},
	{"__unm", value_unm},
	{"__bnot", value_bnot},
	{NULL, NULL}
};

LUAMOD_API int luaopen_value(lua_State *L){
	lua_newtable(L);
	luaL_setfuncs(L, value_f, 0);
	
	/* Create Value metatable */
	if(!luaL_newmetatable(L, "Value")){ // stack: {metatable, table, ...}
		luaL_error(L, "Couldn't create Value metatable");
	}
	
	/* Set metatable */
	lua_pushvalue(L, -2); // duplicate Value table for metamethod upvalue
	luaL_setfuncs(L, value_mt, 1); // put data_mt functions into metatable,
	// add Value table as upvalue
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}
