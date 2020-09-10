#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <unistd.h> // For sleep()

int main(){
	int status;
	
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	if(luaL_loadfile(L, "callbacks.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 0) == LUA_OK){
			printf("[C] Code executed successfully\n");
		}
	}
	
	// Execute callbacks
	for(int i = 0; i < 5; i++){
		sleep(1);
		lua_getglobal(L, "oncustomevent");
		if(!lua_isfunction(L, -1)){
			printf("[C] oncustomevent is not a function\n");
		}
		lua_pushinteger(L, i);
		lua_pcall(L, 1, 0, 0);
		printf("[C] oncustomevent called\n");
	}
	
	lua_close(L);
	return 0;
}