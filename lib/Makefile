.PHONY: tg

tg: libtg.a

libtg.a: ../build/tg.o
	ar rcs $@ $^
../build/tg.o: tg.c tg.h
	$(CC) -o $@ -c $< -lncursesw -fpic