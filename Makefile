CC = gcc
CCFLAGS = -Wall -Werror

all: showFDtables

showFDtables: showFDtables.c
	$(CC) $(CCFLAGS) -o $@ $^

clean:
	rm -f showFDtables 

