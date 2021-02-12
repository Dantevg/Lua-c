/***
 * The `terminal` module provides input autocompletion.
 * This module is basically a nicer `linenoise` binding.
 * @module terminal
 */

#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "linenoise.h"

/* C library definitions */

static lua_State *terminal_L = NULL;

// Autocomplete callback
void terminal_completion(const char *input, linenoiseCompletions *lc){
	if(terminal_L == NULL) return;
	if(lua_getfield(terminal_L, lua_upvalueindex(1), "autocomplete") != LUA_TFUNCTION){
		lua_pop(terminal_L, 1);
		return;
	} // stack: {autocomplete}
	
	lua_pushstring(terminal_L, input); // stack: {input, autocomplete}
	if(lua_pcall(terminal_L, 1, 1, 0) != LUA_OK || !lua_istable(terminal_L, -1)){
		lua_pop(terminal_L, 1);
		return;
	} // stack: {completions}
	
	int n = luaL_len(terminal_L, -1);
	for(int i = 1; i <= n; i++){
		lua_pushstring(terminal_L, input); // stack: {input, completions}
		if(lua_geti(terminal_L, -2, i) == LUA_TSTRING){ // stack: {completion, input, completions}
			lua_concat(terminal_L, 2); // stack: {output, completions}
			linenoiseAddCompletion(lc, lua_tostring(terminal_L, -1));
		}else{
			lua_pop(terminal_L, 1); // stack: {input, completions}
		}
		lua_pop(terminal_L, 1); // stack: {completions}
	}
	
	lua_pop(terminal_L, 1); // stack: {}
}

// Copy the string from lua to the heap
char *copy_hint(lua_State *L, int hint_idx){
	size_t hintlen;
	const char *hint = lua_tolstring(L, hint_idx, &hintlen);
	char *hint_ = (char*)malloc((hintlen+1) * sizeof(char));
	return strncpy(hint_, hint, hintlen+1);
}

// Hints callback
char *terminal_hints(const char *input, int *colour, int *bold){
	if(terminal_L == NULL) return NULL;
	if(lua_getfield(terminal_L, lua_upvalueindex(1), "hints") == LUA_TFUNCTION){
		lua_pushstring(terminal_L, input);
		char *hint = NULL;
		if(lua_pcall(terminal_L, 1, 3, 0) == LUA_OK){
			hint = copy_hint(terminal_L, -3);
			*colour = luaL_optinteger(terminal_L, -2, 90);
			*bold = lua_toboolean(terminal_L, -1);
		}
		lua_pop(terminal_L, 3);
		return hint;
	}else if(lua_getfield(terminal_L, lua_upvalueindex(1), "autocomplete") == LUA_TFUNCTION){
		lua_getfield(terminal_L, lua_upvalueindex(1), "autohints");
		if(lua_toboolean(terminal_L, -1)){
			lua_pop(terminal_L, 1);
			lua_pushstring(terminal_L, input);
			char *hint = NULL;
			if(lua_pcall(terminal_L, 1, 1, 0) == LUA_OK && lua_istable(terminal_L, -1)
					&& lua_geti(terminal_L, -1, 1) == LUA_TSTRING){
				hint = copy_hint(terminal_L, -1);
				*colour = 90;
				*bold = 0;
			}
			lua_pop(terminal_L, 1);
			return hint;
		}
	}
	
	return NULL;
}

/* Lua API definitions */

/***
 * Read input from the user, displaying `prompt` before it.
 * Just like `io.read`, returns `nil` when no input was given.
 * @function read
 * @tparam[opt] string prompt
 * @treturn string|nil
 */
int terminal_read(lua_State *L){
	const char *prompt = luaL_optstring(L, 1, "");
	char *input = linenoise(prompt);
	lua_pushstring(L, input);
	
	/* Auto add to history if wanted (and input was present) */
	if(lua_getfield(L, lua_upvalueindex(1), "history") == LUA_TTABLE && input != NULL){
		lua_getfield(L, -1, "auto");
		if(lua_toboolean(L, -1)){
			linenoiseHistoryAdd(input);
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	
	free(input);
	return 1;
}

/***
 * Clear the screen.
 * @function clear
 */
int terminal_clear(lua_State *L){
	linenoiseClearScreen();
	return 0;
}

/***
 * Add an item to the history.
 * @function history.add
 * @tparam string item
 */
int terminal_history_add(lua_State *L){
	linenoiseHistoryAdd(luaL_checkstring(L, 1));
	return 0;
}

/***
 * Save the history to a file.
 * @function history.save
 * @tparam string path
 * @treturn boolean success
 */
int terminal_history_save(lua_State *L){
	int success = linenoiseHistorySave(luaL_checkstring(L, 1));
	lua_pushboolean(L, success == 0);
	return 1;
}

/***
 * Load history from a file.
 * @function history.load
 * @tparam string path
 * @treturn boolean success
 */
int terminal_history_load(lua_State *L){
	int success = linenoiseHistoryLoad(luaL_checkstring(L, 1));
	lua_pushboolean(L, success == 0);
	return 0;
}

/***
 * Set the history buffer length.
 * Pass 0 (or `nil`) to this function to disable the history buffer.
 * @function history.setLength
 * @tparam[opt=0] int length
 */
int terminal_history_setLength(lua_State *L){
	linenoiseHistorySetMaxLen(luaL_optinteger(L, 1, 0));
	return 0;
}

/***
 * Get the history buffer length.
 * @function history.getLength
 * @treturn int length
 */
int terminal_history_getLength(lua_State *L){
	lua_pushinteger(L, linenoiseHistoryGetMaxLen());
	return 1;
}

/***
 * Autocompletion function.
 * This function receives the input, and should return a table
 * of possible autocompletions.
 * @tfield function autocomplete
 */

/***
 * Hints function.
 * This function receives the input, and should return the hint,
 * optionally followed by the colour (a number from the ANSI escape codes)
 * and a boolean `bold`.
 * @tfield function hints
 */

/***
 * Use autocompletion function for hints, when @{hints} is `nil`.
 * Defaults to `true`
 * @tfield boolean autohints
 */

/***
 * Automatically add items from @{read} to history.
 * Defaults to `true`.
 * @tfield boolean history.auto
 */

static const struct luaL_Reg terminal_f[] = {
	{"read", terminal_read},
	{"clear", terminal_clear},
	{NULL, NULL}
};

static const struct luaL_Reg terminal_history_f[] = {
	{"add", terminal_history_add},
	{"save", terminal_history_save},
	{"load", terminal_history_load},
	{"setLength", terminal_history_setLength},
	{"getLength", terminal_history_getLength},
	{NULL, NULL}
};

LUAMOD_API int luaopen_terminal(lua_State *L){
	luaL_newlibtable(L, terminal_f); // stack: {terminal}
	lua_pushvalue(L, -1);
	luaL_setfuncs(L, terminal_f, 1);
	
	lua_pushboolean(L, 1);
	lua_setfield(L, -2, "autohints");
	
	luaL_newlibtable(L, terminal_history_f); // stack: {terminal.history, terminal}
	lua_pushvalue(L, -1);
	luaL_setfuncs(L, terminal_history_f, 1);
	
	lua_pushboolean(L, 1);
	lua_setfield(L, -2, "auto");
	
	lua_setfield(L, -2, "history"); // stack: {terminal}
	
	// Need to store this for linenoise callbacks to find the Lua state
	terminal_L = L;
	
	linenoiseSetCompletionCallback(terminal_completion);
	linenoiseSetHintsCallback(terminal_hints);
	linenoiseSetFreeHintsCallback(free);
	
	return 1;
}