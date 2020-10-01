#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main(){
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	if(luaL_loadfile(L, "main.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 0) == LUA_OK){
			printf("[C] Code executed successfully\n");
		}
	}
	
	lua_close(L);
	return 0;
}