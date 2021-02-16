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

static int fs_std_close(void *file){
	return fclose(file);
}

static int fs_std_flush(void *file){
	return fflush(file);
}

static size_t fs_std_read(void *file, char *buffer, int n){
	return fread(buffer, sizeof(char), n, file);
}

static char fs_std_getc(void *file){
	return getc(file);
}

static int fs_std_seek(void *file, long offset, int whence){
	return fseek(file, offset, whence);
}

static long fs_std_tell(void *file){
	return ftell(file);
}

static int fs_std_setvbuf(void *file, int mode, size_t size){
	return setvbuf(file, NULL, mode, size);
}

static size_t fs_std_write(void *file, const char *content){
	return fwrite(content, sizeof(char), strlen(content), file);
}

FS fs_std = {
	.open = fs_std_open,
	.close = fs_std_close,
	.flush = fs_std_flush,
	.read = fs_std_read,
	.getc = fs_std_getc,
	.seek = fs_std_seek,
	.tell = fs_std_tell,
	.setvbuf = fs_std_setvbuf,
	.write = fs_std_write,
};

/* Lua API definitions */

LUAMOD_API int luaopen_fs_std(lua_State *L){
	lua_pushlightuserdata(L, &fs_std);
	return 1;
}
