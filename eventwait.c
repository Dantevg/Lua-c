#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <time.h> // For clock()
#include <string.h> // For strcmp()

int main(){
	int status;
	char *eventfilter = "";
	int eventdata = 0;
	clock_t start_time = clock();
	
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	// Load file
	status = luaL_loadfile(L, "eventwait.lua");
	if(status != LUA_OK){
		printf("%s\n", lua_tostring(L, -1));
		return 1;
	}
	
	// Main loop
	do{
		if(eventdata == 0 || (strcmp(eventfilter, "customevent") == 0 && clock() > start_time + eventdata*CLOCKS_PER_SEC)){
			status = lua_resume(L, 0, 0);
			if(status == LUA_YIELD){
				eventfilter = lua_tostring(L, -2);
				eventdata = lua_tointeger(L, -1);
				printf("[C] eventfilter = %s, eventdata = %d\n", eventfilter, eventdata);
			}else if(status != LUA_OK){
				printf("%s\n", lua_tostring(L, -1));
				return 1;
			}
			start_time = clock();
		}
	}while(status != LUA_OK);
	
	lua_close(L);
	return 0;
}