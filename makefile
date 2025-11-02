menu: main.o
	cc -o menu main.o

main.o: main.c
	cc -c main.c

