CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g
MAIN = cdp
OBJS = main.o utility.o operations.o

all : $(MAIN)

$(MAIN) : $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

main.o : main.c utility.h operations.h
	$(CC) $(CFLAGS) -c main.c

operations.o: operations.c utility.h operations.h
	$(CC) $(CFLAGS) -c operations.c

utility.o: utility.c operations.h utility.h 
	$(CC) $(CFLAGS) -c utility.c

clean :
	rm -rf *.o core
