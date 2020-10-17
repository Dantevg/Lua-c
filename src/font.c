#include <SDL.h>

#include "util.c"

typedef struct Char {
	char c;
	SDL_Rect rect;
	int advance;
} Char;

typedef struct Font {
	SDL_Texture *image;
	Char chars[256];
} Font;

Font font_load(SDL_Renderer *renderer, const char *file){
	Font f;
	
	SDL_Surface *surface = SDL_LoadBMP(file);
	checkSDL(surface, "Failed to load image file: %s\n");
	
	f.image = SDL_CreateTextureFromSurface(renderer, surface);
	checkSDL(surface, "Failed to create texture from surface: %s\n");
	
	f.chars['A'].rect.x = 50;
	f.chars['A'].rect.y = 6;
	f.chars['A'].rect.w = 3;
	f.chars['A'].rect.h = 4;
	
	return f;
}

void font_char(SDL_Renderer *renderer, Font *font, SDL_Rect *dest, char c){
	SDL_Rect *src = &font->chars[c].rect;
	dest->w = src->w;
	dest->h = src->h;
	SDL_RenderCopy(renderer, font->image, src, dest);
}