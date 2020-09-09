/*
	Compile with:
	- MacOS: cc main.c -I/usr/local/include/lua -llua
	- Ubuntu: cc main.c -I/usr/include/lua5.3 -llua5.3
*/

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct Player {
	const char* name;
	int level;
};

int main(){
	/* INIT */
	lua_State *L = luaL_newstate(); // Initialise new Lua state
	luaL_openlibs(L); // Load default libs
	luaL_dofile(L, "player.lua"); // Run file
	
	/* RUN */
	lua_getglobal(L, "player1"); // Get player table
	struct Player player; // Create a new player object
	
	// Get name from table
	lua_pushstring(L, "name");
	lua_gettable(L, -2);
	player.name = lua_tostring(L, -1);
	lua_pop(L, 1);
	
	// Get level from table
	lua_pushstring(L, "level");
	lua_gettable(L, -2);
	player.level = lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	printf("Player %s is level %d\n", player.name, player.level);
	
	lua_close(L);
	
	return 0;
}