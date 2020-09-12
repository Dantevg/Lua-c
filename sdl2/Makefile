# Set compiler flags per OS
OS = $(shell uname)
ifeq ($(OS),Darwin) # MacOS
	INCLUDE_DIR = /usr/local/include
else ifeq ($(OS),Linux)
	INCLUDE_DIR = /usr/include
endif

INCLUDE_LUA = -I$(INCLUDE_DIR)/lua -llua5.3
INCLUDE_SDL = -I$(INCLUDE_DIR)/sdl2 -lsdl2

sdl2: sdl2.c
	cc sdl2.c -o ../bin/sdl2 $(INCLUDE_LUA) $(INCLUDE_SDL)