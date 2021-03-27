/***
 * The `safethread` module provides safe multithreading access.
 * 
 * @module safethread
 */

#include <time.h> // for nanosleep
#include <string.h> // for strcmp

#if !defined(_WIN32) && !defined(__WIN32__)
	#define _REENTRANT // needed for pthread_kill
	#include <signal.h> // for SIGINT
#endif
#include "threads.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "MoonBox.h"
#include "safethread.h"
#include "event.h"

/* C library definitions */

// Forward declarations
static int move_value(lua_State*, lua_State*, int);
static int copy_value_(lua_State *from, lua_State *to, int idx, int copiedfrom, int copiedto);

static int try_cached_copy(lua_State *from, lua_State *to, int idx, int copiedfrom, int copiedto){
	lua_pushvalue(from, idx);
	if(lua_gettable(from, copiedfrom) != LUA_TNIL){
		// Found the value, reference it instead of copying
		lua_rawgeti(to, copiedto, lua_tointeger(from, -1));
		lua_pop(from, 1);
		return 1;
	}
	lua_pop(from, 1);
	return 0;
}

static void store_cache(lua_State *from, lua_State *to, int idx, int copiedfrom, int copiedto){
	lua_pushvalue(from, idx);
	lua_pushvalue(to, -1);
	lua_pushinteger(from, luaL_ref(to, copiedto));
	lua_settable(from, copiedfrom);
}

static void copy_table(lua_State *from, lua_State *to, int idx, int copiedfrom, int copiedto){
	/* Check if the table has already been copied (recursive table) */
	if(try_cached_copy(from, to, idx, copiedfrom, copiedto)) return;
	
	lua_newtable(to);

	/* Store new table in "copied" table */
	store_cache(from, to, idx, copiedfrom, copiedto);
	
	/* Traverse table */
	lua_pushnil(from);
	while(lua_next(from, idx) != 0){
		if(copy_value_(from, to, -2, copiedfrom, copiedto)){
			if(copy_value_(from, to, -1, copiedfrom, copiedto)){
				lua_settable(to, -3);
			}else{
				lua_pop(to, 1);
			}
		}
		lua_pop(from, 1);
	}
}

static int writer(lua_State *L, const void *data, size_t size, void *buffer){
	luaL_addlstring((luaL_Buffer *)buffer, (const char *)data, size);
	return 0; // 0 means no errors
}

static int copy_function(lua_State *from, lua_State *to, int idx, int copiedfrom, int copiedto){
	/* C-functions can be copied over simply */
	if(lua_iscfunction(from, idx)){
		lua_pushcfunction(to, lua_tocfunction(from, idx));
		return 1;
	}
	
	/* Check if the function has already been copied */
	if(try_cached_copy(from, to, idx, copiedfrom, copiedto)) return 1;
	
	/* Dump function to buffer */
	luaL_Buffer buffer;
	luaL_buffinit(from, &buffer);
	lua_pushvalue(from, idx);
	lua_dump(from, writer, &buffer, 0);
	luaL_pushresult(&buffer);
	
	/* Retrieve buffer content as string */
	size_t len;
	const char *data = lua_tolstring(from, -1, &len);
	lua_pop(from, 2);
	
	/* Get original function info */
	lua_Debug info;
	lua_pushvalue(from, idx);
	lua_getinfo(from, ">nu", &info);
	
	/* Load buffer back to function */
	int status = luaL_loadbuffer(to, data, len, info.name);
	if(status != LUA_OK){
		fprintf(stderr, "[C] Could not load Lua code: %s\n", lua_tostring(to, -1));
		return 0;
	}
	
	/* Copy upvalues, optionally skip upvalue with name _ENV */
	for(unsigned char i = 1; i <= info.nups; i++){
		const char *name = lua_getupvalue(from, idx, i);
		if(!name) continue;
		if(SHOULD_USE_THREAD_ENV && strcmp(name, "_ENV") == 0){
			lua_rawgeti(to, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		}else{
			if(!copy_value_(from, to, -1, copiedfrom, copiedto)) return 0;
		}
		lua_pop(from, 1);
		if(!lua_setupvalue(to, -2, i)) lua_pop(to, 1);
	}
	
	/* Store new function in "copied" table */
	store_cache(from, to, idx, copiedfrom, copiedto);
	
	return 1;
}

static int copy_value_(lua_State *from, lua_State *to, int idx, int copiedfrom, int copiedto){
	idx = lua_absindex(from, idx);
	int type = lua_type(from, idx);
	switch(type){
		case LUA_TNONE:
		case LUA_TNIL:
			lua_pushnil(to); break;
		case LUA_TBOOLEAN:
			lua_pushboolean(to, lua_toboolean(from, idx)); break;
		case LUA_TLIGHTUSERDATA:
		case LUA_TUSERDATA:
			lua_pushlightuserdata(to, lua_touserdata(from, idx)); break;
		case LUA_TNUMBER:
			lua_pushnumber(to, lua_tonumber(from, idx)); break;
		case LUA_TSTRING: {
			size_t len;
			const char *str = lua_tolstring(from, idx, &len);
			lua_pushlstring(to, str, len); break;
		}
		case LUA_TTABLE:
			copy_table(from, to, idx, copiedfrom, copiedto); break;
		case LUA_TFUNCTION:
			if(!copy_function(from, to, idx, copiedfrom, copiedto)) return 0;
			break;
		case LUA_TTHREAD:
		default:
			return 0;
	}
	
	/* Also copy its metatable (only for tables and userdata),
		but silently fail when that does not work */
	if((type == LUA_TTABLE || type == LUA_TUSERDATA) && lua_getmetatable(from, idx)){
		if(copy_value_(from, to, -1, copiedfrom, copiedto)){
			lua_setmetatable(to, -2);
		}
		lua_pop(from, 1);
	}
	return 1;
}

static int copy_value(lua_State *from, lua_State *to, int idx){
	idx = lua_absindex(from, idx);
	// Create tables to store already copied values
	lua_newtable(from);
	lua_newtable(to);
	int success = copy_value_(from, to, idx, lua_gettop(from), lua_gettop(to));
	// Remove the tables again
	lua_pop(from, 1);
	lua_replace(to, -2);
	return success;
}

static int move_value(lua_State *from, lua_State *to, int idx){
	int success = copy_value(from, to, idx);
	lua_pop(from, 1);
	return success;
}

static void copy_values(lua_State *from, lua_State *to, int n){
	int base = lua_absindex(from, lua_gettop(from)-n+1);
	for(int i = base; i < base+n; i++){
		if(!copy_value(from, to, i)){
			luaL_argerror(from, i, "unsupported type");
		}
	}
}

static void move_values(lua_State *from, lua_State *to, int n){
	copy_values(from, to, n);
	lua_pop(from, n);
}

// Gets called in the new thread
#if defined(_WIN32) || defined(__WIN32__)
DWORD WINAPI safethread_run(LPVOID data){
#else
void *safethread_run(void *data){
#endif
	Thread *t = (Thread*)data;
	lock_mutex(t->mutex); // Immediately lock mutex
	if(lua_gettop(t->L) > 1 && lua_pcall(t->L, lua_gettop(t->L) - 2, LUA_MULTRET, 1) == LUA_OK){
		// Immediately stop execution when main chunk returns false
		// int quit = lua_isboolean(t->L, -1) && lua_toboolean(t->L, -1) == 0;
		
		t->state = THREAD_ACTIVE;
		signal_cond(t->cond);
		
		int quit = 0;
		while(!quit){
			quit = event_loop(t->L);
		}
	}
	
	t->state = THREAD_DEAD;
	unlock_mutex(t->mutex);
	return 0;
}

/* Lua API definitions */

/*** Create a new thread.
 * @function new
 * @tparam[opt] function fn the function to execute in the new thread
 * @param[opt] ... any arguments which will be passed to `fn`
 * @treturn Thread
 */
int safethread_new(lua_State *L){
	/* Create Thread struct / userdata */
	Thread *t = lua_newuserdata(L, sizeof(Thread)); // stack: {t, (args?), fn}
	
	/* Create new Lua state */
	t->L = mb_init();
	
	/* Put Thread struct in registry */
	lua_pushlightuserdata(t->L, t);
	lua_setfield(t->L, LUA_REGISTRYINDEX, "mb_thread");
	
	/* Create hardware mutex and condition variable */
	t->state = THREAD_INIT;
	create_mutex(t->mutex);
	create_cond(t->cond);
	
	/* Load thread chunk */
	if(lua_gettop(L) > 1){
		lua_rotate(L, 1, 1);
		move_values(L, t->L, lua_gettop(L) - 1);
	}
	
	/* Create and start thread */
	create_thread(t->thread, safethread_run, t);
	
	luaL_setmetatable(L, "Thread");
	return 1;
}

/*** Sleep the current thread for an amount of time.
 * Unlike the `os.sleep` provided by MoonBox, this function does not
 * call the event loop, and instead makes the thread idle.
 * @function sleep
 * @tparam number seconds
 */
int safethread_sleep(lua_State *L){
	/* Get self thread */
	lua_getfield(L, LUA_REGISTRYINDEX, "mb_thread");
	Thread *t = lua_touserdata(L, -1);
	lua_pop(L, 1);
	
	lua_Integer microseconds = lua_tonumber(L, 1) * 1000000;
	struct timespec ts;
	ts.tv_sec = microseconds * 1e-6;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	
	unlock_mutex(t->mutex);
	nanosleep(&ts, NULL);
	lock_mutex(t->mutex);
	
	return 0;
}

/*** Exit the current thread.
 * Like `os.exit`, but stops only this thread instead of the whole process.
 * @function exit
 */
int safethread_exit(lua_State *L){
	/* Get self thread */
	lua_getfield(L, LUA_REGISTRYINDEX, "mb_thread");
	Thread *t = lua_touserdata(L, -1);
	lua_pop(L, 1);
	
	t->state = THREAD_DEAD;
	unlock_mutex(t->mutex);
	exit_thread();
	
	return 0;
}

/*** Get the current thread.
 * @function self
 * @treturn Thread the current thread
 */
int safethread_self(lua_State *L){
	lua_getfield(L, LUA_REGISTRYINDEX, "mb_thread");
	luaL_setmetatable(L, "Thread");
	return 1;
}

/// @type Thread

/*** Wait for a thread to complete.
 * @function wait
 * @return the values returned from the thread function
 */
int safethread_wait(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {t}
	if(t->state == THREAD_DEAD) return 0; // Don't wait for a thread that has already stopped
	
	/* Wait for thread and close it */
	join_thread(t->thread);
	destroy_mutex(t->mutex);
	t->state = THREAD_DEAD;
	
	/* Get return values */
	int top = lua_gettop(L);
	move_values(t->L, L, lua_gettop(t->L) - 1);
	
	return lua_gettop(L) - top;
}

/*** Immediately stop a thread.
 * @function kill
 */
int safethread_kill(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {t}
	if(t->state == THREAD_DEAD) return 0; // Don't wait for a thread that has already stopped
	
	/* Send SIGINT to thread */
	kill_thread(t->thread);
	destroy_mutex(t->mutex);
	t->state = 0; // Inactive
	
	return 0;
}

/*** Execute a function in a thread.
 * When the thread is no longer active, returns `nil`.
 * Not all types of function arguments can be copied over. Copyable types are:
 * 
 * - `nil`
 * - `boolean`
 * - `number`
 * - `userdata` (beware of thread synchronisation issues when using the same
 * userdata in multiple threads!)
 * - `string`
 * - `table` (handles recursive / self-referential tables)
 * - `function`
 * 
 * Uncopyable types:
 * 
 * - `thread`
 * 
 * @function pcall
 * @tparam function fn
 * @param[opt] ... args
 * @treturn[1] boolean success
 * @return[1] return values
 * @treturn[2] nil
 */
int safethread_pcall(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {(args?), fn, t}
	if(t->state == THREAD_DEAD){
		// Thread has stopped
		lua_pushboolean(L, 0);
		lua_pushstring(L, "thread has stopped");
		return 2;
	}
	
	luaL_argcheck(L, lua_isfunction(L, 2), 2, "expected function");
	int n_args = lua_gettop(L)-2;
	lock_mutex(t->mutex);
	// Wait until thread has initialised
	while(t->state == THREAD_INIT) wait_cond(t->cond, t->mutex);
	move_values(L, t->L, n_args+1);
	
	int base = lua_gettop(t->L) - n_args - 1;
	if(lua_pcall(t->L, n_args, LUA_MULTRET, 0) != LUA_OK){
		lua_pushboolean(L, 0);
		move_value(t->L, L, -1); // Error will be on top of thread stack
		return 2;
	}
	
	lua_pushboolean(L, 1);
	int n_ret = lua_gettop(t->L) - base;
	move_values(t->L, L, n_ret);
	unlock_mutex(t->mutex);
	return n_ret + 1;
}

/***
 * Push an event on the queue in the thread.
 * @function pushEvent
 * @param name the first event argument
 * @param[opt] ... other event arguments
 */
int safethread_pushEvent(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {(args?), fn, t}
	if(t->state == 0) return 0; // Thread has stopped
	
	lock_mutex(t->mutex);
	lua_pushcfunction(t->L, event_push);
	int n_args = lua_gettop(L)-1;
	move_values(L, t->L, n_args);
	lua_call(t->L, n_args, 0);
	unlock_mutex(t->mutex);
	return 0;
}

static const struct luaL_Reg safethread_f[] = {
	{"new", safethread_new},
	{"sleep", safethread_sleep},
	{"exit", safethread_exit},
	{"self", safethread_self},
	{"wait", safethread_wait},
	{"kill", safethread_kill},
	{"pcall", safethread_pcall},
	{"pushEvent", safethread_pushEvent},
	{NULL, NULL}
};

LUAMOD_API int luaopen_safethread(lua_State *L){
	lua_newtable(L); // stack: {table}
	luaL_setfuncs(L, safethread_f, 0);
	
	/* Create Thread metatable */
	if(!luaL_newmetatable(L, "Thread")){ // stack: {mt, table, ...}
		luaL_error(L, "couldn't create Thread metatable");
	}
	
	// For object-oriented access
	lua_pushvalue(L, -2); // stack: {table, mt, table}
	lua_setfield(L, -2, "__index"); // stack: {mt, table}
	
	// To ensure the threads stop when they go out of scope (e.g. when the program stops)
	lua_pushcfunction(L, safethread_kill); // stack: {safethread_kill, mt, table}
	lua_setfield(L, -2, "__gc"); // stack: {mt, table}
	lua_pop(L, 1); // stack: {table}
	
	int type = lua_getfield(L, LUA_REGISTRYINDEX, "mb_thread");
	lua_pop(L, 1);
	if(type == LUA_TNIL){
		/* mb_thread does not exist in registry, so this thread must be
		the main thread. Create main Thread struct / userdata */
		Thread *t = lua_newuserdata(L, sizeof(Thread)); // stack: {t, table}
		t->state = 1; // Active
		t->L = L;
		t->thread = self_thread();
		create_mutex(t->mutex);
		
		/* Put Thread struct in registry */
		luaL_setmetatable(L, "Thread");
		lua_setfield(L, LUA_REGISTRYINDEX, "mb_thread"); // stack: {table}
	}
	
	return 1;
}
