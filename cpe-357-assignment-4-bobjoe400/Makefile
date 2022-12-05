CC = gcc
ARGS = -Wall -pedantic -std=c99 -g
MAIN = dl
OBJS = main.o utility.o 
all: $(MAIN)

$(MAIN) : $(OBJS)
	$(CC) $(ARGS) -o $(MAIN) $(OBJS)

main.o: main.c utility.h	
	$(CC) $(ARGS) -c main.c

utility.o: utility.c utility.h
	$(CC) $(ARGS) -c utility.c

clean:
	rm -rf *.0 core