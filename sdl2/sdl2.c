#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "screen.c"

struct Window {
	SDL_Window *sdl;
	int width;
	int height;
} window;

SDL_Texture *texture;
lua_State *L;
unsigned int t0;

void resize(){
	
}

int loop(unsigned int dt){
	// Events
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}
	}
	
	// Clear screen
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	// Run lua draw function
	int w, h;
	SDL_GetWindowSize(window.sdl, &w, &h);
	lua_pushinteger(L, w);
	lua_setglobal(L, "width");
	lua_pushinteger(L, h);
	lua_setglobal(L, "height");
	
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
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	
	return 0;
}

int main(){
	// Init Lua
	L = luaL_newstate();
	luaL_openlibs(L);
	
	screen_init(L);
	
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
	window.width = 600;
	window.height = 400;
	window.sdl = SDL_CreateWindow("Test window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window.width, window.height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if(window.sdl == NULL){
		printf("Could not initialize window: %s\n", SDL_GetError());
		return -1;
	}
	
	// Create renderer
	renderer = SDL_CreateRenderer(window.sdl, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if(renderer == NULL){
		printf("Could not initialize renderer: %s\n", SDL_GetError());
		return -1;
	}
	
	// Create texture
	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window.width, window.height);
	if(texture == NULL){
		printf("Could not initialize texture: %s\n", SDL_GetError());
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
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window.sdl);
	lua_close(L);
	renderer = NULL;
	SDL_Quit();
	
	return 0;
}