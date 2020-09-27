#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "screen.c"

struct Window {
	SDL_Window *window;
	SDL_Texture *texture;
	int width;
	int height;
} window;

lua_State *L;
unsigned int t0;

void checkSDL(void *data, char *errstr){
	if(data == NULL){
		printf(errstr, SDL_GetError());
		exit(-1);
	}
}

void resize(){
	// Create a new texture
	SDL_Texture *newtexture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window.width, window.height);
	checkSDL(window.texture, "Could not initialize texture: %s\n");
	
	// Set the source and destination rect
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	SDL_QueryTexture(window.texture, NULL, NULL, &rect.w, &rect.h);
	rect.w = (window.width < rect.w) ? window.width : rect.w;
	rect.h = (window.height < rect.h) ? window.height : rect.h;
	
	// Copy over texture data
	SDL_SetRenderTarget(renderer, newtexture);
	SDL_RenderCopy(renderer, window.texture, &rect, &rect);
	SDL_SetRenderTarget(renderer, NULL);
	SDL_DestroyTexture(window.texture);
	window.texture = newtexture;
}

int loop(unsigned int dt){
	// Events
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 1;
		}else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED){
			window.width = event.window.data1;
			window.height = event.window.data2;
			resize();
		}
	}
	
	// Clear screen
	SDL_SetRenderTarget(renderer, window.texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	// Run lua draw function
	int w, h;
	SDL_GetWindowSize(window.window, &w, &h);
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
	SDL_RenderCopy(renderer, window.texture, NULL, NULL);
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
	window.window = SDL_CreateWindow("Test window",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window.width, window.height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	checkSDL(window.window, "Could not initialize window: %s\n");
	
	// Create renderer
	renderer = SDL_CreateRenderer(window.window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	checkSDL(renderer, "Could not initialize renderer: %s\n");
	
	// Create texture
	window.texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		window.width, window.height);
	checkSDL(window.texture, "Could not initialize texture: %s\n");
	
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
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window.window);
	SDL_Quit();
	lua_close(L);
	
	return 0;
}