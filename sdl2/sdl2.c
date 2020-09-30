#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.c"
#include "screen.c"

struct Timer {
	int id;     // The function id in the lua registry
	int delay;  // The delay in ms
	int repeat; // 1 = repeat, 0 = don't repeat
};

lua_State *L;
unsigned int t0;

uint32_t timer_callback(uint32_t delay, void *param){
	struct Timer *timer = param;
	
	SDL_Event event;
	SDL_UserEvent userevent;
	
	userevent.type = SDL_USEREVENT;
	userevent.code = 1;
	userevent.data1 = timer;
	
	event.type = SDL_USEREVENT;
	event.user = userevent;
	
	// Push timer event on the event stack,
	// to call the lua function in the main thread
	SDL_PushEvent(&event);
	
	// Return the delay to continue repeating, return 0 to stop
	return (timer->repeat) ? delay : 0;
}

int l_addTimer(lua_State *L){
	struct Timer *timer;
	timer->id = luaL_ref(L, LUA_REGISTRYINDEX);
	timer->delay = lua_tointeger(L, -1);
	// timer->repeat = 1;
	
	SDL_AddTimer(timer->delay, timer_callback, timer);
	return 0;
}

int loop(unsigned int dt){
	// Events
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}else if(event.type == SDL_USEREVENT && event.user.code == 1){
			// Got timer event, call Lua callback
			struct Timer *timer = (struct Timer*)event.user.data1;
			lua_rawgeti(L, LUA_REGISTRYINDEX, timer->id);
			lua_pushinteger(L, timer->delay);
			if(lua_pcall(L, 1, 0, 0) != LUA_OK){
				printf("%s\n", lua_tostring(L, -1));
			}
		}
	}
	
	return 0;
}

int main(){
	// Init SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	// Init Lua
	L = luaL_newstate();
	luaL_openlibs(L);
	
	lua_pushcfunction(L, l_addTimer);
	lua_setglobal(L, "addTimer");

	// screen_init(L);

	if(luaL_loadfile(L, "sdl2.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 0) == LUA_OK){
			printf("[C] Code executed successfully\n");
		}else{
			printf("[C] Error in Lua code: %s\n", lua_tostring(L, -1));
			return -1;
		}
	}else{
		printf("[C] Could not load Lua code\n");
		return -1;
	}
	
	// Main loop
	t0 = SDL_GetTicks();
	unsigned int t_prev = t0;
	int quit = 0;
	while(!quit){
		unsigned int t_new = SDL_GetTicks();
		quit = loop(t_new - t_prev);
		t_prev = t_new;
	}
	
	// Exit
	SDL_Quit();
	lua_close(L);
	
	return 0;
}