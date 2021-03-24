/***
 * The `safethread` module provides safe multithreading access.
 * 
 * @module safethread
 */

#include <time.h> // for nanosleep

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

// Gets called in the new thread
#if defined(_WIN32) || defined(__WIN32__)
DWORD WINAPI safethread_run(LPVOID data){
#else
void *safethread_run(void *data){
#endif
	Thread *t = (Thread*)data;
	lock_mutex(t->mutex); // Immediately lock mutex
	mb_run(t->L, 0); // TODO: handle return values
	unlock_mutex(t->mutex);
	return 0;
}

/* Lua API definitions */

/*** Create a new thread.
 * @function new
 * @tparam string path the path to the file to execute in the new thread
 * @param[opt] ... any arguments which will be passed to `fn`
 * @treturn Thread
 */
int safethread_new(lua_State *L){
	const char *path = luaL_checkstring(L, 1);
	
	/* Create Thread struct / userdata */
	Thread *t = lua_newuserdata(L, sizeof(Thread)); // stack: {t, (args?), fn}
	t->state = 1; // Active
	
	/* Create new Lua state */
	t->L = mb_init();
	
	/* Put Thread struct in registry */
	lua_pushlightuserdata(t->L, t);
	lua_setfield(t->L, LUA_REGISTRYINDEX, "mb_thread");
	
	/* Load thread file */
	if(!mb_load(t->L, path)){
		lua_close(t->L);
		return 0;
	}
	
	// TODO: handle arguments
	
	/* Create hardware thread and mutex */
	create_thread(t->thread, safethread_run, t);
	create_mutex(t->mutex);
	
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
	
	lua_Integer microseconds = lua_tonumber(L, 1) * 1e6;
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
	if(t->state == 0) return 0; // Don't wait for a thread that has already stopped
	
	/* Wait for thread and get its return values */
	join_thread(t->thread);
	destroy_mutex(t->mutex);
	// TODO: handle return values
	t->state = 0; // Inactive
	
	return 0;
}

/*** Immediately stop a thread.
 * @function kill
 */
int safethread_kill(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {t}
	if(t->state == 0) return 0; // Don't wait for a thread that has already stopped
	
	/* Send SIGINT to thread */
	kill_thread(t->thread);
	destroy_mutex(t->mutex);
	t->state = 0; // Inactive
	
	return 0;
}

static int copy_value(lua_State *from, lua_State *to, int idx){
	idx = lua_absindex(from, idx);
	switch(lua_type(from, idx)){
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
		case LUA_TSTRING:
			lua_pushstring(to, lua_tostring(from, idx)); break;
		case LUA_TTABLE:
			lua_pushnil(from);
			lua_newtable(to);
			while(lua_next(from, idx) != 0){
				if(copy_value(from, to, -2)){
					if(copy_value(from, to, -1)){
						lua_settable(to, -3);
					}else{
						lua_pop(to, 1);
					}
				}
				lua_pop(from, 1);
			}
			break;
		case LUA_TFUNCTION:
			if(!lua_iscfunction(from, idx)) return 0;
			lua_pushcfunction(to, lua_tocfunction(from, idx)); break;
		case LUA_TTHREAD:
		default:
			return 0;
	}
	return 1;
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

/*** Execute a function in a thread.
 * @function pcall
 * @tparam function fn
 * @param[opt] ... args
 */
int safethread_pcall(lua_State *L){
	/* Get Lua thread */
	Thread *t = luaL_checkudata(L, 1, "Thread"); // stack: {(args?), fn, t}
	if(t->state == 0) return 0; // Thread has stopped
	
	luaL_argcheck(L, lua_iscfunction(L, 2), 2, "only C-functions allowed");
	int n_args = lua_gettop(L)-2;
	lock_mutex(t->mutex);
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
	{"self", safethread_self},
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
