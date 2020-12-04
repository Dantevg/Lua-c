#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "terminal.h"
#include "tg.h"

#define UNICODE_FULL_BLOCK L'\x2588'
#define UNICODE_FULL_BLOCK_STR L"\x2588"

/* C library definitions */

/* Lua API definitions */

// Returns the window width
int termWindow_getWidth(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	lua_pushinteger(L, window->tg->drawBuffer.size.X);
	
	return 1;
}

// Returns the window height
int termWindow_getHeight(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	lua_pushinteger(L, window->tg->drawBuffer.size.Y);
	
	return 1;
}

// Returns the rendering scale
int termWindow_getScale(lua_State *L){
	lua_pushinteger(L, 1);
	return 1;
}

// Sets the rendering scale
int termWindow_setScale(lua_State *L){
	// NOP, for compatibility
	return 0;
}

// Sets drawing colour
int termWindow_colour(lua_State *L){
	// TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	
	// TODO
	
	return 0;
}

// Sets pixel
int termWindow_pixel(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	
	TGBufCursorPosition(&window->tg->drawBuffer, x, y);
	TGBufAddString(&window->tg->drawBuffer, UNICODE_FULL_BLOCK_STR);
	
	return 0;
}

// Draws a rectangle
int termWindow_rect(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	int w = luaL_checkinteger(L, 4);
	int h = luaL_checkinteger(L, 5);
	int fill = lua_toboolean(L, 6);
	
	wchar_t line_full[w+1];
	for(int i = 0; i < w; i++){
		line_full[i] = UNICODE_FULL_BLOCK;
	}
	line_full[w] = '\0';
	
	if(fill){
		for(int dy = 0; dy < h; dy++){
			TGBufCursorPosition(&window->tg->drawBuffer, x, y+dy);
			TGBufAddString(&window->tg->drawBuffer, line_full);
		}
	}else{
		wchar_t line_mid[w+1];
		for(int i = 1; i < w-1; i++){
			line_mid[i] = ' ';
		}
		line_mid[0] = UNICODE_FULL_BLOCK;
		line_mid[w-1] = UNICODE_FULL_BLOCK;
		line_mid[w] = '\0';
		
		TGBufCursorPosition(&window->tg->drawBuffer, x, y);
		TGBufAddString(&window->tg->drawBuffer, line_full);
		for(int dy = 1; dy < h-1; dy++){
			TGBufCursorPosition(&window->tg->drawBuffer, x, y+dy);
			TGBufAddString(&window->tg->drawBuffer, line_mid);
		}
		TGBufCursorPosition(&window->tg->drawBuffer, x, y+h-1);
		TGBufAddString(&window->tg->drawBuffer, line_full);
	}
	
	return 0;
}

// Clears the termWindow canvas using the current colour
int termWindow_clear(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	TGBufClear(&window->tg->drawBuffer);
	return 0;
}

int termWindow_char(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	const char *str = luaL_checkstring(L, 2);
	int x = luaL_checkinteger(L, 3);
	int y = luaL_checkinteger(L, 4);
	
	TGBufCursorPosition(&window->tg->drawBuffer, x, y);
	TGCharInfo c;
	c.character = str[0];
	TGBufCell(&window->tg->drawBuffer, x, y, c);
	
	return 0;
}

int termWindow_write(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	const char *str = luaL_checkstring(L, 2);
	int x = luaL_checkinteger(L, 3);
	int y = luaL_checkinteger(L, 4);
	
	TGBufCursorPosition(&window->tg->drawBuffer, x, y);
	TGBufAddLString(&window->tg->drawBuffer, str);
	
	return 0;
}

int termWindow_loadFont(lua_State *L){
	// NOP, for compatibility
	return 0;
}

// Resizes the termWindow canvas
// Intended to be used as callback (ignores first argument, event name)
int termWindow_resize(lua_State *L){
	TermWindow *window = luaL_checkudata(L, 1, "TermWindow");
	int w = luaL_checkinteger(L, 3);
	int h = luaL_checkinteger(L, 4);
	
	TGBufSize(&window->tg->drawBuffer, w, h);
	
	return 0;
}

int termWindow_present(lua_State *L){
	TGUpdate();
	return 0;
}

int termWindow__gc(lua_State *L){
	TGEnd();
	return 0;
}

int termWindow_new(lua_State *L){
	/* Create termWindow */
	TermWindow *window = lua_newuserdata(L, sizeof(TermWindow)); // stack: {TermWindow}
	window->tg = TG();
	
	/* Return window, termWindow userdata is still on stack */
	luaL_setmetatable(L, "TermWindow"); // Set the termWindow metatable to the userdata
	return 1;
}

static const struct luaL_Reg termWindow_f[] = {
	{"getWidth", termWindow_getWidth},
	{"getHeight", termWindow_getHeight},
	{"getScale", termWindow_getScale},
	{"setScale", termWindow_setScale},
	{"colour", termWindow_colour},
	{"pixel", termWindow_pixel},
	{"rect", termWindow_rect},
	{"clear", termWindow_clear},
	{"char", termWindow_char},
	{"write", termWindow_write},
	{"loadFont", termWindow_loadFont},
	{"resize", termWindow_resize},
	{"present", termWindow_present},
	{"new", termWindow_new},
	{NULL, NULL}
};

LUAMOD_API int luaopen_screen_terminal(lua_State *L){
	lua_newtable(L); // stack: {table, ...}
	luaL_setfuncs(L, termWindow_f, 0);
	
	/* Create termWindow metatable */
	if(!luaL_newmetatable(L, "TermWindow")){ // stack: {metatable, table, ...}
		luaL_error(L, "couldn't create TermWindow metatable");
	}
	
	/* Set __index to termWindow, for OO */
	lua_pushvalue(L, -2); // stack: {table, metatable, table, ...}
	lua_setfield(L, -2, "__index"); // stack: {metatable, table, ...}
	
	// To ensure the TG library stops when it goes out of scope
	lua_pushcfunction(L, termWindow__gc); // stack: {termWindow__gc, mt, table}
	lua_setfield(L, -2, "__gc"); // stack: {mt, table}
	lua_pop(L, 1); // stack: {table, ...}
	
	return 1;
}