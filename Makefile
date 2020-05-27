CC = g++
LD = g++
CFLAGS = -pthread
LDFLAGS = -pthread -o

PROGS = client server

all:		$(PROGS)

client:	client.cpp
		$(CC) $< $(LDFLAGS) $@

server:	server.cpp
		$(CC) $< $(LDFLAGS) $@

clean:
		rm -f $(PROGS)
