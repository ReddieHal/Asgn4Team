all: mytar
	
mytar: mytar.o tarcreate.o tarextract.o helper.o helper.h
	gcc -o mytar -lm helper.h helper.o tarcreate.o tarextract.o mytar.o
mytar.o: mytar.c
	gcc -Werror -Wall -pedantic -g -c mytar.c
tarcreate.o: tarcreate.c
	gcc -Werror -Wall -pedantic -g -c tarcreate.c
tarextract.o: tarextract.c 
	gcc -Werror -Wall -pedantic -g -c tarextract.c
helper.o: helper.c
	gcc -Werror -Wall -pedantic -g -c helper.c
clean:
	rm *.o
