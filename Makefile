# Set compiler flags per OS
ifeq ($(OS),macos)
	args:= -I/usr/local/include/lua -llua
else ifeq ($(OS),ubuntu)
	args := -I/usr/include/lua5.3 -llua5.3
else # assume ubuntu
	args := -I/usr/include/lua5.3 -llua5.3
endif

main: main.c
	cc main.c -o bin/main $(args)

player: player.c
	cc player.c -o bin/player $(args)

callbacks: callbacks.c
	cc callbacks.c -o bin/callbacks $(args)

eventwait: eventwait.c
	cc eventwait.c -o bin/eventwait $(args)