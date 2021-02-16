#pragma once

#include <lua.h>

#include "../fs.h"

FS fs_std;

LUAMOD_API int luaopen_fs_std(lua_State *L);
