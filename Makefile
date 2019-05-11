PROG=	sh
SRCS=	sh.c

CFLAGS=		-O2 -fstack-protector -D_FORTIFY_SOURCE=2 -pie -fPIE
LDFLAGS=	-Wl,-z,now -Wl,-z,relro

$(PROG): $(SRCS)
	gcc $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PROG).out

debug: $(SRCS)
	gcc -g -DDEBUG -Wall -Wextra -Wconversion $(SRCS) -o $(PROG).out

clean:
	rm -f $(PROG).out
