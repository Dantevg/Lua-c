#include <lua.h>

#include "table.h"

void table_create(lua_State *L, int autodecrement){
	lua_newtable(L);
	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "n");
	lua_pushboolean(L, autodecrement);
	lua_setfield(L, -2, "autodecrement");
}

int table_get_autodecrement(lua_State *L, int idx){
	lua_getfield(L, idx, "autodecrement");
	int autodecrement = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return autodecrement;
}

int table_getn(lua_State *L, int idx){
	lua_getfield(L, idx, "n");
	int n = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return n;
}

void table_setn(lua_State *L, int idx, int n){
	idx = lua_absindex(L, idx);
	lua_pushinteger(L, n);
	lua_setfield(L, idx, "n");
}

int table_insert(lua_State *L, int idx){
	int n = table_getn(L, idx) + 1;
	lua_seti(L, idx, n);
	table_setn(L, idx, n);
	return n;
}

void table_remove(lua_State *L, int idx, int n){
	idx = lua_absindex(L, idx);
	n = (n > 0) ? n : table_getn(L, idx) + n + 1;
	lua_geti(L, idx, n);
	lua_pushnil(L);
	lua_seti(L, idx, n);
	if(n == table_getn(L, idx) && table_get_autodecrement(L, idx)){
		table_setn(L, idx, n-1);
	}
}
