CC = gcc
ARGS = -Wall -pedantic -std=c99 -g
MAIN = tt
OBJS = main.o utility.o #commands.o

all: $(MAIN)

$(MAIN) : $(OBJS)
	$(CC) $(ARGS) -o $(MAIN) $(OBJS)

main.o: main.c utility.h	
	$(CC) $(ARGS) -c main.c

#commands.o: commands.c commands.h utility.h
#	$(CC) $(ARGS) -c commands.c

utility.o: utility.c utility.h
	$(CC) $(ARGS) -c utility.c

clean:
	rm -rf *.0 core