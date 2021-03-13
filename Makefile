CFLAGS = -Wall -Wextra -Wshadow -Wno-unused-parameter -std=gnu99 -DBASE_PATH=\"$(CURDIR)/\" -g
LUA_VERSION = 5.3
INCLUDE = -I/usr/include/lua$(LUA_VERSION) -I/usr/local/include/lua$(LUA_VERSION) -isystem lib
LIBS_MAIN = -llua$(LUA_VERSION)
LIBS_SO = -lSDL2 -llua$(LUA_VERSION)
SO = so

# Parallel compilation is faster
MAKEFLAGS += -j

libs = bin/event.$(SO)\
	bin/SDLWindow.$(SO)\
	bin/image/SDLImage.$(SO)\
	bin/sys.$(SO)\
	bin/mouse.$(SO)\
	bin/kb.$(SO)\
	bin/Buffer.$(SO)\
	bin/value.$(SO)\
	bin/thread.$(SO)\
	bin/fs.$(SO)\
	bin/fs/std.$(SO)

libs_posix = bin/screen/terminal.$(SO)\
	bin/terminal.$(SO)

libs_win = 

ifeq ($(OS),Windows_NT)
	CFLAGS += -DLUA_LIB -DLUA_BUILD_AS_DLL
	# TODO: use $HOME or other more generic env var?
	INCLUDE = -I "C:\Program Files\Lua53\include" -isystem lib
	LIBS_MAIN = -llua
	LIBS_SO = -lmingw32 -lSDL2main -lSDL2 -llua
	SO = dll
	libs += $(libs_win)
else
	libs += $(libs_posix)
endif

.PHONY: all init main libraries clean

all: main libraries
init:
	mkdir -p build bin
	mkdir -p build/image bin/image build/thread bin/thread build/screen bin/screen build/fs bin/fs
main: bin/MoonBox
libraries: $(libs)



# Dependency list

bin/MoonBox: build/main.o build/event.o build/util.o build/table.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS_SO)
build/main.o: src/main.c src/event.c src/event.h src/util.c src/util.h

build/util.o: src/util.c src/util.h

build/font.o: src/font.c src/font.h

build/table.o: src/table.c src/table.h

bin/event.$(SO): build/event.o build/util.o build/table.o
build/event.o: src/event.c src/event.h

bin/SDLWindow.$(SO): build/SDLWindow.o build/font.o build/util.o
build/SDLWindow.o: src/SDLWindow.c src/SDLWindow.h

bin/image/SDLImage.$(SO): build/image/SDLImage.o build/font.o build/util.o
build/image/SDLImage.o: src/image/SDLImage.c src/image/SDLImage.h

bin/thread.$(SO): build/thread.o
build/thread.o: src/thread.c src/thread.h src/threads.h

bin/sys.$(SO): build/sys.o
build/sys.o: src/sys.c

bin/mouse.$(SO): build/mouse.o
build/mouse.o: src/mouse.c src/mouse.h

bin/kb.$(SO): build/kb.o
build/kb.o: src/kb.c src/kb.h

bin/Buffer.$(SO): build/Buffer.o
build/Buffer.o: src/Buffer.c src/Buffer.h

bin/value.$(SO): build/value.o
build/value.o: src/value.c src/value.h

bin/terminal.$(SO): src/terminal.c lib/linenoise.c lib/linenoise.h
	$(CC) $(CFLAGS) -std=gnu99 $(INCLUDE) -shared -fpic -o $@ lib/linenoise.c src/terminal.c $(LIBS_MAIN)

bin/fs.$(SO): build/fs.o
build/fs.o: src/fs.c src/fs.h

bin/fs/std.$(SO): build/fs/std.o build/fs.o
build/fs/std.o: src/fs/std.c src/fs/std.h src/fs.c src/fs.h

bin/screen/terminal.$(SO): build/screen/terminal.o lib/libtg.a build/event.o build/util.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS_SO) -shared -lncursesw
build/screen/terminal.o: src/screen/terminal.c src/screen/terminal.h src/event.c src/event.h src/util.c src/util.h



# Automatic rules

# For libraries (.so/.dll files)
bin/%.$(SO): build/%.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS_SO) -shared

# For intermediate files (.o files)
build/%.o: src/%.c
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDE) -fpic

clean:
	rm -r build
	rm -r bin
