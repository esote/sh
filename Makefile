sh: sh.c
	gcc -O2 -o sh.out sh.c

debug: sh.c
	gcc -g -DDEBUG -Wall -Wextra -Wconversion -o sh.out sh.c

clean:
	rm -f sh.out
