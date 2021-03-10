#pragma once

#include <lua.h>

void table_create(lua_State *L, int autodecrement);
int table_get_autodecrement(lua_State *L, int idx);
int table_getn(lua_State *L, int idx);
void table_setn(lua_State *L, int idx, int n);
int table_insert(lua_State *L, int idx);
void table_remove(lua_State *L, int idx, int n);
