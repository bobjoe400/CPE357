CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g
CLIENT = client
CLIENT_OBJS = client.o
SERVER = httpd
SERVER_OBJS = server.o net.o utility.o
PROGS = $(CLIENT) $(SERVER)

all : $(PROGS)

$(CLIENT) : $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJS)

client.o : client.c
	$(CC) $(CFLAGS) -c client.c

$(SERVER) : $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJS)

server.o : server.c net.h utility.h
	$(CC) $(CFLAGS) -c server.c

net.o : net.c net.h
	$(CC) $(CFLAGS) -c net.c

utility.o: utility.c utility.h
	$(CC) $(CFLAGS) -c utility.c

clean :
	rm *.o $(PROGS) core
