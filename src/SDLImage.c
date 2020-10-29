#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "SDLImage.h"
#include "font.h"

#define SDLImage_BITDEPTH    32
#define SDLImage_PIXELFORMAT SDL_PIXELFORMAT_RGB888

/* C library definitions */

SDLImage *SDLImage_get(lua_State *L){
	void *p = lua_touserdata(L, 1);
	if(p == NULL) luaL_argerror(L, 1, "expected SDLImage userdata");
	return (SDLImage*)p;
}

void SDLImage_create(SDLImage *image, int w, int h){
	image->rect.x = 0;
	image->rect.y = 0;
	image->rect.w = w;
	image->rect.h = h;
	
	/* Create surface */
	image->surface = SDL_CreateRGBSurfaceWithFormat(0, image->rect.w, image->rect.h,
		SDLImage_BITDEPTH, SDLImage_PIXELFORMAT);
	checkSDL(image->surface, "Could not initialize surface: %s\n");
}

void SDLImage_load(SDLImage *image, const char *filename){
	image->rect.x = 0;
	image->rect.y = 0;
	
	/* Load surface */
	image->surface = SDL_LoadBMP(filename);
	checkSDL(image->surface, "Could not load image: %s\n");
	
	image->rect.w = image->surface->w;
	image->rect.h = image->surface->h;
}

/* Lua API definitions */

// Returns the image width
int SDLImage_getWidth(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	lua_pushinteger(L, image->rect.w / image->scale);
	
	return 1;
}

// Returns the image height
int SDLImage_getHeight(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	lua_pushinteger(L, image->rect.h / image->scale);
	
	return 1;
}

// Returns the rendering scale
int SDLImage_getScale(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	lua_pushinteger(L, image->scale);
	
	return 1;
}

// Sets the rendering scale
int SDLImage_setScale(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	image->scale = luaL_checkinteger(L, 2);
	SDL_RenderSetScale(image->renderer, image->scale, image->scale);
	
	return 0;
}

// Sets drawing colour
int SDLImage_colour(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	int r = luaL_checkinteger(L, 2);
	int g = luaL_optinteger(L, 3, r);
	int b = luaL_optinteger(L, 4, r);
	int a = luaL_optinteger(L, 5, 255);
	
	SDL_SetRenderDrawColor(image->renderer, r, g, b, a);
	
	return 0;
}

// Sets pixel
int SDLImage_pixel(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	SDL_RenderDrawPoint(image->renderer, x, y);
	
	return 0;
}

// Draws a rectangle
int SDLImage_rect(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	
	SDL_Rect rect;
	rect.x = luaL_checkinteger(L, 2);
	rect.y = luaL_checkinteger(L, 3);
	rect.w = luaL_checkinteger(L, 4);
	rect.h = luaL_checkinteger(L, 5);
	int fill = lua_toboolean(L, 6);
	
	if(fill){
		SDL_RenderFillRect(image->renderer, &rect);
	}else{
		SDL_RenderDrawRect(image->renderer, &rect);
	}
	
	return 0;
}

// Clears the image using the current colour
int SDLImage_clear(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	SDL_RenderClear(image->renderer);
	
	return 0;
}

// Draws a character on the image
int SDLImage_char(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	const char *str = luaL_checkstring(L, 2);
	SDL_Rect rect;
	rect.x = luaL_checkinteger(L, 3);
	rect.y = luaL_checkinteger(L, 4);
	font_char(&image->font, image->renderer, &rect, str[0]);
	
	return 0;
}

// Draws a string of characters on the image
int SDLImage_write(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	const char *str = luaL_checkstring(L, 2);
	
	/* Get string length */
	lua_len(L, 2);
	int n = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	/* Set initial position rect */
	int x = luaL_checkinteger(L, 3);
	int y = luaL_checkinteger(L, 4);
	SDL_Rect rect;
	
	/* Draw characters */
	for(int i = 0; i < n; i++){
		rect.x = x;
		rect.y = y;
		x += font_char(&image->font, image->renderer, &rect, str[i]);
	}
	return 0;
}

// Gets the pixel colour on the given coordinates
int SDLImage_getPixel(lua_State *L){
	SDLImage *image = SDLImage_get(L); // stack: {y, x, SDLImage}
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	/* Check coordinates */
	if(x < 0 || y < 0 || x >= image->rect.w || y >= image->rect.h){
		luaL_error(L, "Coordinates out of range");
	}
	
	/* Get pixel */
	int pitch = image->surface->pitch;
	uint8_t bpp = image->surface->format->BytesPerPixel;
	uint8_t *p = (uint8_t*)image->surface->pixels + y * pitch + x * bpp;
	
	/* Get colour */
	uint8_t r, g, b;
	SDL_GetRGB(*(uint32_t*)p, image->surface->format, &r, &g, &b);
	
	/* Return colour */
	lua_pushinteger(L, r);
	lua_pushinteger(L, g);
	lua_pushinteger(L, b);
	return 3;
}

// Loads a font
int SDLImage_loadFont(lua_State *L){
	SDLImage *image = SDLImage_get(L); // stack: {filename, SDLImage}
	lua_replace(L, 1); // stack: {filename}
	image->font = font_load(L, image->renderer);
	return 0;
}

// Resizes the image
// Intended to be used as callback for screen interface compatibility
// (ignores first argument, event name)
int SDLImage_resize(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	
	image->rect.w = luaL_checkinteger(L, 3);
	image->rect.h = luaL_checkinteger(L, 4);
	
	/* Create new surface */
	SDL_Surface *newsurface = SDL_CreateRGBSurfaceWithFormat(0, image->rect.w, image->rect.h,
		SDLImage_BITDEPTH, SDLImage_PIXELFORMAT);
	checkSDL(newsurface, "Could not initialize surface: %s\n");
	
	/* Set the source and destination rect */
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = (image->rect.w < image->surface->w) ? image->rect.w : image->surface->w;
	rect.h = (image->rect.h < image->surface->h) ? image->rect.h : image->surface->h;
	
	/* Copy over surface data */
	SDL_BlitSurface(image->surface, &rect, newsurface, &rect);
	SDL_DestroyRenderer(image->renderer);
	SDL_FreeSurface(image->surface);
	image->surface = newsurface;
	
	/* Create new renderer */
	image->renderer = SDL_CreateSoftwareRenderer(image->surface);
	checkSDL(image->renderer, "Could not initialize renderer: %s\n");
	SDL_RenderSetScale(image->renderer, image->scale, image->scale);
	
	return 0;
}

// For screen interface compatibility
int SDLImage_present(lua_State *L){
	return 0;
}

// Saves the image to a file
int SDLImage_save(lua_State *L){
	SDLImage *image = SDLImage_get(L);
	const char *filename = luaL_checkstring(L, 2);
	
	if(SDL_SaveBMP(image->surface, filename) != 0){
		/* Failure, return false and erorr message */
		lua_pushboolean(L, 0);
		lua_pushstring(L, SDL_GetError());
		return 2;
	}
	
	/* Success, return true */
	lua_pushboolean(L, 1);
	return 1;
}

int SDLImage_new(lua_State *L){
	SDLImage *image;
	
	if(lua_type(L, 1) == LUA_TSTRING){ // stack: {filename}
		/* Load image from file */
		const char *filename = luaL_checkstring(L, 1);
		
		image = lua_newuserdata(L, sizeof(SDLImage)); // stack: {SDLImage, filename}
		SDLImage_load(image, filename);
		
	}else if(lua_type(L, 1) == LUA_TNUMBER){ // stack: {h, w}
		/* Create new image */
		int w = luaL_checkinteger(L, 1);
		int h = luaL_checkinteger(L, 2);
		
		image = lua_newuserdata(L, sizeof(SDLImage)); // stack: {SDLImage, w, h}
		SDLImage_create(image, w, h);
		
	}else{
		luaL_error(L, "Expected string or number, number");
	}
	
	image->scale = 1;
	
	/* Create renderer */
	image->renderer = SDL_CreateSoftwareRenderer(image->surface);
	checkSDL(image->renderer, "Could not initialize renderer: %s\n");
	
	/* Set default colour to white */
	SDL_SetRenderDrawColor(image->renderer, 255, 255, 255, 255);
	
	/* Return image, SDLImage userdata is still on stack */
	luaL_setmetatable(L, "SDLImage"); // Set the SDLImage metatable to the userdata
	return 1;
}

int luaopen_SDLImage(lua_State *L){
	lua_newtable(L); // stack: {table, ...}
	luaL_setfuncs(L, SDLImage_f, 0);
	
	/* Create SDLImage metatable */
	if(!luaL_newmetatable(L, "SDLImage")){ // stack: {metatable, table, ...}
		luaL_error(L, "Couldn't create SDLImage metatable");
	}
	
	/* Set __index to SDLImage, for OO */
	lua_pushvalue(L, -2); // stack: {table, metatable, table, ...}
	lua_setfield(L, -2, "__index"); // stack: {metatable, table, ...}
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}