/***
 * The `std` fs module wraps the default file system
 * @module std
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../fs.h"

#include "std.h"

/* C library definitions */

FS fs_std;

static void *fs_std_open(const char *path, const char *mode){
	return fopen(path, mode);
}

static void fs_std_close(void *file){
	fclose(file);
}

static void fs_std_flush(void *file){
	fflush(file);
}

static int fs_std_read(void *file, char *buffer, int n){
	return fread(buffer, sizeof(char), n, file);
}

static char fs_std_getc(void *file){
	return getc(file);
}

static int fs_std_seek(void *file, int base, int amount){
	int status = fseek(file, amount, base);
	return (status == 0) ? ftell(file) : -1;
}

static void fs_std_setvbuf(void *file, int mode, int size){
	setvbuf(file, NULL, mode, size);
}

static void fs_std_write(void *file, const char *content){
	fwrite(content, sizeof(char), strlen(content), file);
}

FS fs_std = {
	.open = fs_std_open,
	.close = fs_std_close,
	.flush = fs_std_flush,
	.read = fs_std_read,
	.getc = fs_std_getc,
	.seek = fs_std_seek,
	.setvbuf = fs_std_setvbuf,
	.write = fs_std_write,
};

/* Lua API definitions */

LUAMOD_API int luaopen_fs_std(lua_State *L){
	lua_pushlightuserdata(L, &fs_std);
	return 1;
}
