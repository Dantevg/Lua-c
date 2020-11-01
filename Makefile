CFLAGS = -Wall -Wextra -Wno-unused-parameter
INCLUDE = -I/usr/include/lua5.3
LIBS = -lSDL2 -llua5.3

ifeq ($(shell uname),Darwin) # MacOS
	# Maybe instead use pkg-config, not sure (https://stackoverflow.com/a/52954005/3688140)
	INCLUDE = -I/usr/local/include/lua5.3
endif

.PHONY: all main libraries clean

all: main libraries
main: bin/main
libraries: bin/event.so bin/SDLWindow.so bin/SDLImage.so bin/thread.so bin/sys.so bin/mouse.so



# Dependency list

bin/main: build/main.o build/util.o
build/main.o: src/main.c src/main.h

build/util.o: src/util.c src/util.h

build/font.o: src/font.c src/font.h

bin/event.so: build/event.o build/util.o
build/event.o: src/event.c src/event.h

bin/SDLWindow.so: build/SDLWindow.o build/font.o build/util.o
build/SDLWindow.o: src/SDLWindow.c src/SDLWindow.h

bin/SDLImage.so: build/SDLImage.o build/font.o build/util.o
build/SDLImage.o: src/SDLImage.c src/SDLImage.h

bin/thread.so: build/thread.o build/util.o
build/thread.o: src/thread.c src/thread.h

bin/sys.so: build/sys.o
build/sys.o: src/sys.c

bin/mouse.so: build/mouse.o
build/mouse.o: src/mouse.c src/mouse.h



# Automatic rules

# For main file
bin/%: build/%.o
	cc $^ -o $@ $(CFLAGS) $(LIBS)

# For libraries (.so files)
bin/%.so: build/%.o
	cc $^ -o $@ $(CFLAGS) $(LIBS) -shared

# For intermediate files (.o files)
build/%.o: src/%.c
	cc -c $< -o $@ $(CFLAGS) $(INCLUDE) -fPIC

clean:
	rm build/*
	rm bin/*