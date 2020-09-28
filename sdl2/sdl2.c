#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.c"
#include "screen.c"

lua_State *L;
unsigned int t0;

int loop(unsigned int dt){
	// Events
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED){
			window.width = event.window.data1;
			window.height = event.window.data2;
			screen_resize();
		}
	}
	
	// Clear screen
	SDL_SetRenderTarget(window.renderer, window.texture);
	SDL_SetRenderDrawColor(window.renderer, 0, 0, 0, 255);
	SDL_RenderClear(window.renderer);
	SDL_RenderSetScale(window.renderer, window.scale, window.scale);
	
	// Run lua draw function
	lua_getglobal(L, "draw");
	if(lua_isfunction(L, -1)){
		lua_pushinteger(L, dt);
		if(lua_pcall(L, 1, 0, 0) != LUA_OK){
			printf("%s\n", lua_tostring(L, -1));
		}
	}else{
		lua_pop(L, 1); // Pop draw function, because lua_pcall isn't called
	}
	
	// Display
	SDL_SetRenderTarget(window.renderer, NULL);
	SDL_RenderCopy(window.renderer, window.texture, NULL, NULL);
	SDL_RenderPresent(window.renderer);
	
	return 0;
}

int main(){
	// Init Lua
	L = luaL_newstate();
	luaL_openlibs(L);
	
	
	// Init SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	screen_init(L);

	if(luaL_loadfile(L, "sdl2.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 0) == LUA_OK){
			printf("[C] Code executed successfully\n");
		}
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
	SDL_DestroyTexture(window.texture);
	SDL_DestroyRenderer(window.renderer);
	SDL_DestroyWindow(window.window);
	SDL_Quit();
	lua_close(L);
	
	return 0;
}