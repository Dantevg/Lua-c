#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

SDL_Window *window;
SDL_Renderer *renderer;

int l_colour(lua_State *L){
	int r = lua_tointeger(L, -4);
	int g = lua_tointeger(L, -3);
	int b = lua_tointeger(L, -2);
	int a = lua_tointeger(L, -1);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	
	return 0;
}

int l_pixel(lua_State *L){
	lua_getglobal(L, "scale");
	int scale = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	int x = lua_tointeger(L, -2);
	int y = lua_tointeger(L, -1);
	
	SDL_RenderSetScale(renderer, scale, scale);
	SDL_RenderDrawPoint(renderer, x, y);
	
	return 0;
}

int main(){
	// Init Lua
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	lua_pushcfunction(L, l_pixel);
	lua_setglobal(L, "pixel");
	
	lua_pushcfunction(L, l_colour);
	lua_setglobal(L, "colour");
	
	lua_pushinteger(L, 600);
	lua_setglobal(L, "width");
	
	lua_pushinteger(L, 400);
	lua_setglobal(L, "height");
	
	if(luaL_loadfile(L, "sdl2.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 0) == LUA_OK){
			printf("[C] Code executed successfully\n");
		}
	}
	
	// Init SDL
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	// Create window
	window = SDL_CreateWindow("Test window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		600, 400,
		SDL_WINDOW_SHOWN);
	if(window == NULL){
		printf("Could not initialize window: %s\n", SDL_GetError());
		return -1;
	}
	
	// Create renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL){
		printf("Could not initialize renderer: %s\n", SDL_GetError());
		return -1;
	}
	
	int quit = 0;
	SDL_Event e;
	unsigned int t = SDL_GetTicks();
	
	while(quit != 1){
		// Events
		while(SDL_PollEvent(&e) != 0){
			if(e.type == SDL_QUIT){
				quit = 1;
			}
		}
		
		// Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		
		// Run lua draw function
		lua_getglobal(L, "draw");
		if(lua_isfunction(L, -1)){
			lua_pushinteger(L, SDL_GetTicks()-t);
			if(lua_pcall(L, 1, 0, 0) != LUA_OK){
				printf("%s\n", lua_tostring(L, -1));
			}
		}else{
			lua_pop(L, 1); // Pop draw function, because lua_pcall isn't called
		}
		
		// Display
		SDL_RenderPresent(renderer);
	}
	
	// Exit
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	lua_close(L);
	renderer = NULL;
	window = NULL;
	SDL_Quit();
	
	return 0;
}