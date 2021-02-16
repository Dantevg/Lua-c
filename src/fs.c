/***
 * The `fs` module provides access to MoonBox's filesystem
 * @module fs
 * 
 * Parts of this code are taken from Lua's liolib.c:
 * https://github.com/lua/lua/blob/master/liolib.c
 */

#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "fs.h"

/* C library definitions */

void fs_mount(lua_State *L, const char *path, FS *fs){
	lua_getfield(L, LUA_REGISTRYINDEX, "fs_filesystems");
	lua_pushlightuserdata(L, fs);
	lua_setfield(L, -2, path);
	lua_pop(L, 1);
}

char *copy_str(lua_State *L, int idx){
	size_t length;
	const char *str = lua_tolstring(L, idx, &length);
	if(str == NULL) return NULL;
	char *str_ = (char*)malloc((length+1) * sizeof(char));
	return strncpy(str_, str, length+1);
}

int fs_match_base(const char *a, const char *b){
	
	return 1;
}

FS_File *fs_getfile(lua_State *L){
	FS_File *file = luaL_checkudata(L, 1, "File");
	if(!file->opened) luaL_error(L, "attempt to use a closed file");
	return file;
}

/* Lua API definitions */

int fs_mount_l(lua_State *L){
	char *path = copy_str(L, 1);
	if(lua_isuserdata(L, 2)){
		fs_mount(L, path, lua_touserdata(L, 2));
		lua_pushboolean(L, 1);
	}else{
		lua_pushboolean(L, 0);
	}
	return 1;
}

int fs_openwith(lua_State *L, const char *path, const char *mode, FS *fs){
	FS_File *file = lua_newuserdata(L, sizeof(FS_File));
	file->opened = 1;
	file->fs = fs;
	file->fd = fs->open(path, mode);
	luaL_setmetatable(L, "File");
	return (file->fd == NULL) ? luaL_fileresult(L, 0, path) : 1;
}

int fs_open(lua_State *L){
	const char *path = luaL_checkstring(L, 1);
	const char *mode = luaL_optstring(L, 2, "r");
	lua_getfield(L, LUA_REGISTRYINDEX, "fs_filesystems");
	int t = lua_gettop(L);
	lua_pushnil(L);
	while(lua_next(L, t) != 0){
		if(fs_match_base(path, lua_tostring(L, -2))){
			return fs_openwith(L, path, mode, lua_touserdata(L, -1));
		}
		lua_pop(L, 1);
	}
	return 0;
}

int fs_close(lua_State *L){
	FS_File *file = fs_getfile(L);
	file->opened = 0;
	file->fs->close(file->fd);
	lua_pushboolean(L, 1);
	return 1;
}

int fs_flush(lua_State *L){
	FS_File *file = fs_getfile(L);
	return luaL_fileresult(L, file->fs->flush(file->fd) == 0, NULL);
}

int fs_lines(lua_State *L){
	return 0;
}

int fs_read(lua_State *L){
	FS_File *file = fs_getfile(L);
	int n = luaL_checkinteger(L, 2);
	char *buffer = malloc(n * sizeof(char));
	int n_read = file->fs->read(file->fd, buffer, n);
	if(n_read > 0){
		lua_pushlstring(L, buffer, n_read);
	}else{
		lua_pushnil(L);
	}
	free(buffer);
	return 1;
}

// Adapted from https://github.com/lua/lua/blob/c03c527fd207b4ad8f5a8e0f4f2c176bd227c979/liolib.c#L696
int fs_seek(lua_State *L){
	static const int modes[] = {SEEK_SET, SEEK_CUR, SEEK_END};
	static const char *const modenames[] = {"set", "cur", "end", NULL};
	FS_File *file = fs_getfile(L);
	int whence = luaL_checkoption(L, 2, "cur", modenames);
	lua_Integer p3 = luaL_optinteger(L, 3, 0);
	long offset = (long)p3;
	luaL_argcheck(L, (lua_Integer)offset == p3, 3, "not an integer in proper range");
	int result = file->fs->seek(file->fd, offset, modes[whence]);
	if(result != 0){
		return luaL_fileresult(L, 0, NULL);
	}else{
		lua_pushinteger(L, file->fs->tell(file->fd));
		return 1;
	}
}

// Adapted from https://github.com/lua/lua/blob/c03c527fd207b4ad8f5a8e0f4f2c176bd227c979/liolib.c#L715
int fs_setvbuf(lua_State *L){
	static const int modes[] = {_IONBF, _IOFBF, _IOLBF};
	static const char *const modenames[] = {"no", "full", "line", NULL};
	FS_File *file = fs_getfile(L);
	int mode = luaL_checkoption(L, 2, NULL, modenames);
	size_t size = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
	int res = file->fs->setvbuf(file->fd, modes[mode], size);
	return luaL_fileresult(L, res == 0, NULL);
}

int fs_write(lua_State *L){
	return 0;
}

int fs__gc(lua_State *L){
	FS_File *file = luaL_checkudata(L, 1, "File");
	if(file->opened) file->fs->close(file->fd);
	return 0;
}

static const struct luaL_Reg fs_f[] = {
	{"mount", fs_mount_l},
	{"open", fs_open},
	{NULL, NULL}
};

static const struct luaL_Reg fs_file_f[] = {
	{"close", fs_close},
	{"flush", fs_flush},
	{"lines", fs_lines},
	{"read", fs_read},
	{"seek", fs_seek},
	{"setvbuf", fs_setvbuf},
	{"write", fs_write},
	{NULL, NULL}
};

static const struct luaL_Reg fs_file_m[] = {
	{"__gc", fs__gc},
	{NULL, NULL}
};

LUAMOD_API int luaopen_fs(lua_State *L){
	luaL_newlib(L, fs_f);
	
	luaL_newmetatable(L, "File");
	luaL_newlib(L, fs_file_f);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, fs_file_m, 0);
	lua_pop(L, 1);
	
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "fs_filesystems");
	
	return 1;
}