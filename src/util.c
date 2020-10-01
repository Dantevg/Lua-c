#include <stddef.h>

void checkSDL(void *data, char *errstr){
	if(data == NULL){
		printf(errstr, SDL_GetError());
		exit(-1);
	}
}