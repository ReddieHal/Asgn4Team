all: mytar
	
mytar: mytar.o helper.o helper.h
	gcc -o mytar helper.h helper.o mytar.o
mytar.o: mytar.c
	gcc -Werror -Wall -pedantic -g -c mytar.c
helper.o: helper.c
	gcc -Werror -Wall -pedantic -g -c helper.c
clean:
	rm *.o
