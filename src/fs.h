#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct FS FS;

typedef struct FS_File {
	void *fd;
	FS *fs;
} FS_File;

typedef struct FS {
	void* (*open)(const char*, const char*);
	
	void (*close)(void*);
	void (*flush)(void*);
	int (*read)(void*, char*, int);
	char (*getc)(void*);
	int (*seek)(void*, int, int);
	void (*setvbuf)(void*, int, int);
	void (*write)(void*, const char*);
} FS;

/* Lua API definitions */

LUAMOD_API int luaopen_fs(lua_State *L);