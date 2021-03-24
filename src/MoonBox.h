#pragma once

#include <time.h>

#include <lua.h>
#include <lauxlib.h>

#define VERSION "0.3.0"

#ifndef BASE_PATH
	#define BASE_PATH "/"
#endif

#if defined(_WIN32)
	#define SO_EXT "dll"
#else
	#define SO_EXT "so"
#endif

int mb_error_handler(lua_State *L);
int mb_os_clock(lua_State *L);
lua_State *mb_init();
int mb_load(lua_State *L, const char *file);
int mb_run(lua_State *L, int n_args, int loop);
void mb_main(lua_State *L, const char *file, int n_args);
