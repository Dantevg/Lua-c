/***
 * @script moonbox
 * @usage
 * Usage: moonbox [options] [file [args]]
 * Execute FILE, or the default boot file
 * 
 * Options:
 *   -v, --version      print version
 *   -h, --help         print this help message
 *   -m, --module name  require library 'name'. Pass '*' to load all available
 *   -e chunk           execute 'chunk'
 *   -                  stop handling options and execute stdin
 */

#include <string.h> // for strcmp
#include <stdlib.h> // for exit

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "MoonBox.h"

void print_usage(){
	printf("Usage: moonbox [options] [file [args]]\n");
	printf("Execute 'file', or the default boot file\n\n");
	printf("Options:\n" );
	printf("  -v, --version\t\tprint version\n");
	printf("  -h, --help\t\tprint this help message\n");
	printf("  -m, --module name\trequire library 'name'. Pass '*' to load all available\n");
	printf("  -e chunk\t\texecute 'chunk'\n");
	printf("  -\t\t\tstop handling options and execute stdin\n");
}

void parse_cmdline_args(int argc, char *argv[], char **file, int *lua_arg_start, lua_State *L){
	int stop = 0;
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0){
			/* Print version and return */
			printf("MoonBox " VERSION "\n");
			stop = 1;
		}else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			/* Print usage and return */
			print_usage();
			stop = 1;
		}else if(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--module") == 0){
			/* Load module */
			if(i >= argc){
				fprintf(stderr, "Option --module expects an argument");
				exit(EXIT_FAILURE);
			}
			char *module = argv[++i];
			lua_getglobal(L, "require");
			lua_pushstring(L, module);
			if(lua_pcall(L, 1, 1, 0) != LUA_OK){
				fprintf(stderr, "[C] Could not load module %s:\n%s\n", module, lua_tostring(L, -1));
				exit(EXIT_FAILURE);
			}
			lua_setglobal(L, module); // TODO: Set better name for submodules
		}else if(strcmp(argv[i], "-e") == 0){
			/* Execute command-line argument */
			if(i >= argc){
				fprintf(stderr, "Option -e expects an argument");
				exit(EXIT_FAILURE);
			}
			char *code = argv[++i];
			if(luaL_loadstring(L, code) == LUA_OK){
				lua_pcall(L, 0, 0, 1);
				stop = 1;
			}else{
				fprintf(stderr, "[C] Could not load Lua code: %s\n", lua_tostring(L, -1));
				exit(EXIT_FAILURE);
			}
		}else if(strcmp(argv[i], "-") == 0){
			/* Execute stdin */
			*file = NULL;
			return;
		}else if(argv[i][0] == '-'){
			/* Unrecognised command line option */
			fprintf(stderr, "Unrecognised option: %s\n", argv[i]);
			print_usage();
			exit(EXIT_FAILURE);
		}else{
			/* Execute file */
			*file = argv[i];
			*lua_arg_start = i+1;
			return;
		}
	}
	
	if(stop){
		lua_close(L);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[]){
	/* Init Lua */
	lua_State *L = mb_init();
	
	/* Parse command-line arguments */
	char *file = BASE_PATH "res/main.lua";
	int lua_arg_start = argc;
	parse_cmdline_args(argc, argv, &file, &lua_arg_start, L);
	
	/* Push lua args as strings and run */
	for(int i = lua_arg_start; i < argc; i++){
		lua_pushstring(L, argv[i]);
	}
	mb_main(L, file, argc-lua_arg_start);
	
	lua_close(L);
	return 0;
}