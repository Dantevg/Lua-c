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

# Aliases
.PHONY: all modules main screen event

all: main modules
modules: bin/screen.so bin/event.so
main: bin/main
screen: bin/screen.so
event: bin/event.so

# Main file
bin/main: src/main.c
	cc src/main.c -o bin/main $(INCLUDE_LUA) $(INCLUDE_SDL)

# Modules: bin/screen.so -> cc src/screen.c -o bin/screen.so (...)
bin/%.so: src/%.c
	cc $< -o $@ $(INCLUDE_LUA) $(INCLUDE_SDL) -shared -fPIC