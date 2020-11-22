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

#if defined(_WIN32)
	#define SO_EXT "dll"
#else
	#define SO_EXT "so"
#endif

void print_usage(){
	printf("Usage: moonbox [options] [file [args]]\n");
	printf("Execute FILE, or the default boot file\n\n");
	printf("Options:\n" );
	printf("  -v, --version\tprint version\n");
	printf("  -h, --help\tprint this help message\n");
	printf("  -\t\tstop handling options and execute stdin\n");
}

int main(int argc, char *argv[]){
	char *file = "res/main.lua";
	int lua_arg_start = argc;
	
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
		}else if(strcmp(argv[i], "-") == 0){
			/* Execute stdin */
			file = NULL;
			break;
		}else if(argv[i][0] == '-'){
			/* Unrecognised command line option */
			printf("Unrecognised option: %s\n", argv[i]);
			print_usage();
			return -1;
		}else{
			/* Execute file */
			file = argv[i];
			lua_arg_start = i+1;
			break;
		}
	}
	
	/* Init Lua */
	lua_State *L = luaL_newstate();
	luaL_openlibs(L); // Open standard libraries (math, string, table, ...)
	
	/* Set cpath and path */
	if(luaL_dostring(L, "package.cpath = package.cpath..';./bin/?." SO_EXT "'")){
		fprintf(stderr, "[C] Could not set package.cpath: %s\n", lua_tostring(L, -1));
	}
	if(luaL_dostring(L, "package.path = package.path..';./res/lib/?.lua'")){
		fprintf(stderr, "[C] Could not set package.path: %s\n", lua_tostring(L, -1));
	}
	
	/* Load main file */
	if(luaL_loadfile(L, file) == LUA_OK){
		// chdir("res");
		/* Push lua args */
		for(int i = lua_arg_start; i < argc; i++){
			lua_pushstring(L, argv[i]);
		}
		if(lua_pcall(L, argc-lua_arg_start, 1, 0) == LUA_OK){
			// Immediately stop execution when main chunk returns false
			if(lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0){
				return 0;
			}
		}else{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
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