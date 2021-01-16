/***
 * @script moonbox
 * @usage
 * Usage: moonbox [options] [file [args]]
 * Execute FILE, or the default boot file
 * 
 * Options:
 *   -v, --version  print version
 *   -h, --help     print this help message
 *   -              stop handling options and execute stdin
 */

#include <unistd.h> // For chdir
#include <string.h> // for strcmp

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define VERSION "0.2.0"

#ifndef BASE_PATH
	#define BASE_PATH "/"
#endif

#if defined(_WIN32)
	#define SO_EXT "dll"
#else
	#define SO_EXT "so"
#endif

void print_usage(){
	printf("Usage: moonbox [options] [file [args]]\n");
	printf("Execute 'file', or the default boot file\n\n");
	printf("Options:\n" );
	printf("  -v, --version\t\tprint version\n");
	printf("  -h, --help\t\tprint this help message\n");
	printf("  -m, --module name\trequire library 'name'. Pass '*' to load all available\n");
	printf("  -\t\t\tstop handling options and execute stdin\n");
}

int lua_error_handler(lua_State *L){
	luaL_traceback(L, L, lua_tostring(L, -1), 2);
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
	return 1;
}

int main(int argc, char *argv[]){
	char *file = BASE_PATH "res/main.lua";
	int lua_arg_start = argc;
	
	/* Init Lua */
	lua_State *L = luaL_newstate();
	luaL_openlibs(L); // Open standard libraries (math, string, table, ...)
	
	/* Set cpath and path */
	if(luaL_dostring(L, "package.cpath = package.cpath..';" BASE_PATH "bin/?." SO_EXT "'")){
		fprintf(stderr, "[C] Could not set package.cpath:\n%s\n", lua_tostring(L, -1));
	}
	if(luaL_dostring(L, "package.path = package.path..';" BASE_PATH "res/lib/?.lua'")){
		fprintf(stderr, "[C] Could not set package.path:\n%s\n", lua_tostring(L, -1));
	}
	
	/* Push Lua error handler */
	lua_pushcfunction(L, lua_error_handler);
	
	/* Parse command-line arguments */
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0){
			/* Print version and return */
			printf("MoonBox " VERSION "\n");
			return 0;
		}else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			/* Print usage and return */
			print_usage();
			return 0;
		}else if(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--module") == 0){
			if(i >= argc){
				fprintf(stderr, "Option --module expects an argument");
				return -1;
			}
			char *module = argv[++i];
			lua_getglobal(L, "require");
			lua_pushstring(L, module);
			if(lua_pcall(L, 1, 1, 0) != LUA_OK){
				fprintf(stderr, "[C] Could not load module %s:\n%s\n", module, lua_tostring(L, -1));
				return -1;
			}
			lua_setglobal(L, module);
		}else if(strcmp(argv[i], "-") == 0){
			/* Execute stdin */
			file = NULL;
			break;
		}else if(argv[i][0] == '-'){
			/* Unrecognised command line option */
			fprintf(stderr, "Unrecognised option: %s\n", argv[i]);
			print_usage();
			return -1;
		}else{
			/* Execute file */
			file = argv[i];
			lua_arg_start = i+1;
			break;
		}
	}
	
	/* Run init file */
	if(luaL_loadfile(L, BASE_PATH "res/init.lua") == LUA_OK){
		if(lua_pcall(L, 0, 0, 1) != LUA_OK) return -1;
	}else{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return -1;
	}
	
	/* Load main file */
	if(luaL_loadfile(L, file) == LUA_OK){
		/* Push lua args */
		for(int i = lua_arg_start; i < argc; i++){
			lua_pushstring(L, argv[i]);
		}
		if(lua_pcall(L, argc-lua_arg_start, 1, 1) == LUA_OK){
			// Immediately stop execution when main chunk returns false
			if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
				return 0;
			}
		}else{
			// Error message was already printed by lua_error_handler
			return -1;
		}
	}else{
		fprintf(stderr, "[C] Could not load Lua code: %s\n", lua_tostring(L, -1));
		return -1;
	}
	
	/* Main loop */
	lua_getfield(L, LUA_REGISTRYINDEX, "event_loop");
	if(lua_islightuserdata(L, -1)){
		int (*event_loop_ptr)(lua_State*) = lua_touserdata(L, -1);
		lua_pop(L, 1);
		if(event_loop_ptr != NULL){
			int quit = 0;
			while(!quit){
				quit = event_loop_ptr(L);
			}
		}
	}
	
	/* Exit */
	lua_close(L);
	
	return 0;
}