#include <SDL2/SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.h"
#include "font.h"

void font_transfer_int_field(lua_State *L, int i, const char *name, int *dst){
	lua_getfield(L, -1, name);
	if(!lua_isinteger(L, -1)){
		luaL_error(L, "Char number %d doesn't contain integer field '%s'", i, name);
	}
	(*dst) = lua_tointeger(L, -1);
	lua_pop(L, 1);
}

// Loads a font from a fontname_meta.lua with accompanying fontname.bmp
Font font_load(lua_State *L, SDL_Renderer *renderer){
	/* Load meta file */
	const char *metafile = luaL_checkstring(L, 1);
	if(luaL_loadfile(L, metafile) != LUA_OK){
		luaL_error(L, "Couldn't load file: %s", lua_tostring(L, -1));
	}
	if(lua_pcall(L, 0, 1, 0) != LUA_OK){ // stack: {fonttable / error, metafile}
		luaL_error(L, "Couldn't load file: %s", lua_tostring(L, -1));
	}
	if(!lua_istable(L, -1)){ // File should be of the form "return {...}"
		luaL_error(L, "Font file doesn't return table");
	}
	
	/* Get image file path */
	// lua_pushstring(L, "res/"); // stack: {"res/", fonttable, metafile}
	lua_getfield(L, -1, "file"); // stack: {fonttable.file, "res/", fonttable, metafile}
	if(!lua_isstring(L, -1)){
		luaL_error(L, "Font file doesn't contain string field 'file'");
	}
	// lua_concat(L, 2); // stack: {"res/"..fonttable.file, fonttable, metafile}
	const char *imagefile = lua_tostring(L, -1);
	lua_pop(L, 1); // stack: {fonttable, metafile}
	
	/* Check chars table */
	lua_getfield(L, -1, "chars"); // stack: {charstable, fonttable, metafile}
	if(!lua_istable(L, -1)){
		luaL_error(L, "Font file doesn't contain table field 'chars'");
	}
	
	/* Load image into surface */
	SDL_Surface *surface = SDL_LoadBMP(imagefile);
	checkSDL(surface, "Failed to load image file: %s\n");
	
	Font f;
	
	/* Convert surface to texture */
	f.image = SDL_CreateTextureFromSurface(renderer, surface);
	checkSDL(surface, "Failed to create texture from surface: %s\n");
	
	/* Get font height */
	lua_getfield(L, -2, "height");
	if(!lua_isinteger(L, -1)){
		luaL_error(L, "Font file doesn't contain string field 'height'");
	}
	f.height = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	/* Populate chars */
	lua_len(L, -1);
	int n = lua_tointeger(L, -1);
	lua_pop(L, 1);
	
	for(int i = 1; i <= n; i++){
		lua_geti(L, -1, i); // stack: {chartable, charstable, fonttable, metafile}
		if(!lua_istable(L, -1)){
			luaL_error(L, "Char number %d is not a table", i);
		}
		
		/* Char.c */
		lua_getfield(L, -1, "char");
		if(!lua_isstring(L, -1)){
			luaL_error(L, "Char number %d doesn't contain string field 'char'", i);
		}
		char c = lua_tostring(L, -1)[0];
		f.chars[(int)c].c = c;
		lua_pop(L, 1);
		
		/* Other integer character metrics */
		font_transfer_int_field(L, c, "x", &f.chars[(int)c].rect.x);
		font_transfer_int_field(L, c, "y", &f.chars[(int)c].rect.y);
		font_transfer_int_field(L, c, "ox", &f.chars[(int)c].ox);
		font_transfer_int_field(L, c, "oy", &f.chars[(int)c].oy);
		font_transfer_int_field(L, c, "width", &f.chars[(int)c].rect.w);
		font_transfer_int_field(L, c, "height", &f.chars[(int)c].rect.h);
		font_transfer_int_field(L, c, "advance", &f.chars[(int)c].advance);
		
		lua_pop(L, 1); // stack: {charstable, fonttable, metafile}
	}
	
	return f;
}

// Renders a character on the given coordinates
int font_char(Font *font, SDL_Renderer *renderer, SDL_Rect *dest, char c){
	/* Set rect */
	SDL_Rect *src = &font->chars[(int)c].rect;
	dest->x += font->chars[(int)c].ox;
	dest->y += font->height - font->chars[(int)c].rect.h - font->chars[(int)c].oy;
	dest->w = src->w;
	dest->h = src->h;
	
	/* Set colour */
	uint8_t r;
	uint8_t g;
	uint8_t b;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, NULL);
	SDL_SetTextureColorMod(font->image, r, g, b);
	
	/* Render */
	SDL_RenderCopy(renderer, font->image, src, dest);
	return font->chars[(int)c].advance;
}