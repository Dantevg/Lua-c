#pragma once

#include <SDL2/SDL.h>

typedef struct Timer {
	int id;     // The SDL timer ID
	int delay;  // The delay in ms
	int repeat; // 1 = repeat, 0 = don't repeat
} Timer;

typedef struct Callback {
	int id;            // The callback id in the callback table
	int fn;            // The function id in the Lua registry
	void *data;        // Optional extra data
	const char *event; // The event name
} Callback;

// Store the event names corresponding to the SDL2 event enum
const static struct {
	SDL_EventType event;
	const char *name;
} events[] = {
	{SDL_QUIT,            "quit"},
	{SDL_KEYDOWN,         "kb.down"},
	{SDL_KEYUP,           "kb.up"},
	{SDL_TEXTINPUT,       "kb.input"},
	{SDL_MOUSEMOTION,     "mouse.move"},
	{SDL_MOUSEBUTTONDOWN, "mouse.down"},
	{SDL_MOUSEBUTTONUP,   "mouse.up"},
	{SDL_MOUSEWHEEL,      "mouse.scroll"}
};

void (*event_dispatch_ptr)(lua_State*, SDL_Event*);