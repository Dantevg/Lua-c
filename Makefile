CFLAGS = -Wall -Wextra -Wshadow -Wno-unused-parameter -std=c99
INCLUDE = -I/usr/include/lua5.3
LIBS = -lSDL2 -llua5.3
SO = so

ifeq ($(shell uname),Darwin) # MacOS
	# Maybe instead use pkg-config, not sure (https://stackoverflow.com/a/52954005/3688140)
	INCLUDE = -I/usr/local/include/lua5.3
else ifeq ($(OS),Windows_NT)
	CFLAGS += -Dmain=SDL_main -DLUA_LIB -DLUA_BUILD_AS_DLL
	INCLUDE = -I "C:\Users\dante\Documents\mingw\include\lua" -Dmain=SDL_main
	LIBS = -lmingw32 -lSDL2main -lSDL2 -llua
	SO = dll
endif

.PHONY: all main libraries clean

all: main libraries
main: bin/main
libraries: bin/event.$(SO)\
	bin/SDLWindow.$(SO)\
	bin/SDLImage.$(SO)\
	bin/thread.$(SO)\
	bin/sys.$(SO)\
	bin/mouse.$(SO)\
	bin/kb.$(SO)



# Dependency list

bin/main: build/main.o build/util.o
build/main.o: src/main.c src/main.h

build/util.o: src/util.c src/util.h

build/font.o: src/font.c src/font.h

bin/event.$(SO): build/event.o build/util.o
build/event.o: src/event.c src/event.h

bin/SDLWindow.$(SO): build/SDLWindow.o build/font.o build/util.o
build/SDLWindow.o: src/SDLWindow.c src/SDLWindow.h

bin/SDLImage.$(SO): build/SDLImage.o build/font.o build/util.o
build/SDLImage.o: src/SDLImage.c src/SDLImage.h

bin/thread.$(SO): build/thread.o build/util.o
build/thread.o: src/thread.c src/thread.h

bin/sys.$(SO): build/sys.o
build/sys.o: src/sys.c

bin/mouse.$(SO): build/mouse.o
build/mouse.o: src/mouse.c src/mouse.h

bin/kb.$(SO): build/kb.o
build/kb.o: src/kb.c src/kb.h



# Automatic rules

# For main file
bin/%: build/%.o
	cc $^ -o $@ $(CFLAGS) $(LIBS)

# For libraries (.so/.dll files)
bin/%.$(SO): build/%.o
	cc $^ -o $@ $(CFLAGS) $(LIBS) -shared

# For intermediate files (.o files)
build/%.o: src/%.c
	cc -c $< -o $@ $(CFLAGS) $(INCLUDE) -fpic

clean:
	rm build/*
	rm bin/*