/***
 * The `safethread` module provides safe multithreading access.
 * 
 * @module safethread
 */

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
	lua_State *L = (lua_State*)data;
	mb_run(L, 0); // TODO: handle return values
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
	create_thread(&t->thread, safethread_run, t->L);
	create_mutex(&t->mutex);
	lock_mutex(&t->mutex); // Immediately lock mutex
	
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
	destroy_mutex(&t->mutex);
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
	destroy_mutex(&t->mutex);
	t->state = 0; // Inactive
	
	return 0;
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
	
	lock_mutex(&t->mutex);
	lua_pushcfunction(t->L, event_push);
	int n_args = lua_gettop(L)-1;
	move_values(L, t->L, n_args);
	lua_call(t->L, n_args, 0);
	unlock_mutex(&t->mutex);
	return 0;
}

static const struct luaL_Reg safethread_f[] = {
	{"new", safethread_new},
	{"wait", safethread_wait},
	{"kill", safethread_kill},
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
	
	return 1;
}
