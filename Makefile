.PHONY: clean


all: main.o
	cc -o main main.c -g -pthread
clean :
	rm *.o

run :
	./sort $(word)
