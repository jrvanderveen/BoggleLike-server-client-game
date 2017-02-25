CC = gcc
CFLAGS = -Wall -c
LDFLAGS = -g
CLIENTOBJS = prog2_client.c clientUtils.c
SERVEROBJS = prog2_server.c trie.c serverUtils.c
EXES = prog2_server prog2_client

all:	$(EXES)
prog2_client:	$(CLIENTOBJS)
	$(CC) -o $@ $(LDFLAGS) $(CLIENTOBJS)
prog2_server:	$(SERVEROBJS)
	$(CC) -o $@ $(LDFLAGS) $(SERVEROBJS)
%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -f *.o $(EXES)
