# Set compiler flags per OS
OS = $(shell uname)
ifeq ($(OS),Darwin) # MacOS
	INCLUDE_DIR = /usr/local/include
else ifeq ($(OS),Linux)
	INCLUDE_DIR = /usr/include
endif

INCLUDE_LUA = -I$(INCLUDE_DIR)/lua -llua5.3
INCLUDE_SDL = -I$(INCLUDE_DIR)/sdl2 -lsdl2

main: main.c
	cc main.c -o bin/main $(INCLUDE_LUA)

player: player.c
	cc player.c -o bin/player $(INCLUDE_LUA)

callbacks: callbacks.c
	cc callbacks.c -o bin/callbacks $(INCLUDE_LUA)

eventwait: eventwait.c
	cc eventwait.c -o bin/eventwait $(INCLUDE_LUA)

sdl2: sdl2.c
	cc sdl2.c -o bin/sdl2 $(INCLUDE_LUA) $(INCLUDE_SDL)