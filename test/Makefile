# Set compiler flags per OS
OS = $(shell uname)
ifeq ($(OS),Darwin) # MacOS
	INCLUDE_DIR = /usr/local/include
	INCLUDE_LUA = -I$(INCLUDE_DIR)/lua -llua5.3
	INCLUDE_SDL = -I$(INCLUDE_DIR)/sdl2 -lsdl2
else ifeq ($(OS),Linux)
	INCLUDE_DIR = /usr/include
	INCLUDE_LUA = -I$(INCLUDE_DIR)/lua5.3 -llua5.3
	INCLUDE_SDL = -I$(INCLUDE_DIR)/SDL2 -lSDL2
endif

main: main.c
	cc main.c -o ../bin/main $(INCLUDE_LUA)

player: player.c
	cc player.c -o ../bin/player $(INCLUDE_LUA)

callbacks: callbacks.c
	cc callbacks.c -o ../bin/callbacks $(INCLUDE_LUA)

eventwait: eventwait.c
	cc eventwait.c -o ../bin/eventwait $(INCLUDE_LUA)