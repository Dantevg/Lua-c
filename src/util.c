#include <SDL2/SDL.h>

#include "util.h"

void checkSDL(void *data, char *errstr){
	if(data == NULL){
		fprintf(stderr, errstr, SDL_GetError());
		exit(-1);
	}
}

void lower(const char *str, char *out, int length){
	for(int i = 0; i < length; i++){
		out[i] = tolower(str[i]);
	}
}