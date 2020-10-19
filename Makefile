CFLAGS = -Wall
INCLUDE = -I/usr/include/lua5.3
LIBS = -lSDL2 -llua5.3

ifeq ($(shell uname),Darwin) # MacOS
	# Maybe instead use pkg-config, not sure (https://stackoverflow.com/a/52954005/3688140)
	INCLUDE = -I/usr/local/include/lua5.3
endif

.PHONY: all clean

all: bin/main bin/event.so bin/screen.so

# Normal files
bin/main: build/main.o build/util.o
build/main.o: src/main.c src/main.h

# Libraries
bin/event.so: build/event.o
	cc build/event.o -o bin/event.so $(CFLAGS) $(LIBS) -shared

build/event.o: src/event.c src/event.h
	cc -c src/event.c -o build/event.o $(CFLAGS) $(INCLUDE) -fPIC

bin/screen.so: build/screen.o build/font.o build/util.o
	cc build/screen.o build/font.o build/util.o -o bin/screen.so $(CFLAGS) $(LIBS) -shared

build/screen.o: src/screen.c src/screen.h
	cc -c src/screen.c -o build/screen.o $(CFLAGS) $(INCLUDE) -fPIC

build/font.o: src/font.c src/font.h
	cc -c src/font.c -o build/font.o $(CFLAGS) $(INCLUDE) -fPIC

# Automatic (fallback) rules
bin/%: build/%.o
	cc $< -o $@ $(CFLAGS) $(LIBS)

bin/%.so: build/%.o
	cc $< -o $@ $(CFLAGS) $(LIBS) -shared

build/%.o: src/%.c
	cc -c $< -o $@ $(CFLAGS) $(INCLUDE)

clean:
	rm build/*
	rm bin/*