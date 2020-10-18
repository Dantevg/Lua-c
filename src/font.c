#include <SDL.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "util.c"

typedef struct Char {
	char c;
	SDL_Rect rect;
	int ox;
	int oy;
	int advance;
} Char;

typedef struct Font {
	SDL_Texture *image;
	int height;
	Char chars[256];
} Font;

void font_transfer_int_field(lua_State *L, int i, const char *name, int *dst){
	lua_getfield(L, -1, name);
	if(!lua_isinteger(L, -1)){
		luaL_error(L, "Char number %d doesn't contain integer field '%s'", i, name);
	}
	(*dst) = lua_tointeger(L, -1);
	lua_pop(L, 1);
}

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
	lua_pushstring(L, "res/"); // stack: {"res/", fonttable, metafile}
	lua_getfield(L, -2, "file"); // stack: {fonttable.file, "res/", fonttable, metafile}
	if(!lua_isstring(L, -1)){
		luaL_error(L, "Font file doesn't contain string field 'file'");
	}
	lua_concat(L, 2); // stack: {"res/"..fonttable.file, fonttable, metafile}
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
		f.chars[c].c = c;
		lua_pop(L, 1);
		
		/* Other integer character metrics */
		font_transfer_int_field(L, c, "x", &f.chars[c].rect.x);
		font_transfer_int_field(L, c, "y", &f.chars[c].rect.y);
		font_transfer_int_field(L, c, "ox", &f.chars[c].ox);
		font_transfer_int_field(L, c, "oy", &f.chars[c].oy);
		font_transfer_int_field(L, c, "width", &f.chars[c].rect.w);
		font_transfer_int_field(L, c, "height", &f.chars[c].rect.h);
		font_transfer_int_field(L, c, "advance", &f.chars[c].advance);
		
		lua_pop(L, 1); // stack: {charstable, fonttable, metafile}
	}
	
	return f;
}

int font_char(SDL_Renderer *renderer, Font *font, SDL_Rect *dest, char c){
	SDL_Rect *src = &font->chars[c].rect;
	dest->x += font->chars[c].ox;
	dest->y += font->height - font->chars[c].rect.h - font->chars[c].oy;
	dest->w = src->w;
	dest->h = src->h;
	SDL_RenderCopy(renderer, font->image, src, dest);
	return font->chars[c].advance;
}