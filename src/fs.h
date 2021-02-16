#pragma once

#include <lua.h>
#include <lauxlib.h>

/* C library definitions */

typedef struct FS FS;

typedef struct FS_File {
	int opened;
	void *fd;
	FS *fs;
} FS_File;

typedef struct FS {
	void* (*open)(const char *path, const char *mode);
	
	int (*close)(void *file);
	int (*flush)(void *file);
	size_t (*read)(void *file, char *buffer, int n);
	char (*getc)(void *file);
	int (*seek)(void *file, long offset, int whence);
	long (*tell)(void *file);
	int (*setvbuf)(void *file, int mode, size_t size);
	size_t (*write)(void *file, const char *content);
} FS;

/* Lua API definitions */

LUAMOD_API int luaopen_fs(lua_State *L);