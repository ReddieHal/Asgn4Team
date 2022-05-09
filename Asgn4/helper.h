#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

#define BLOCKSIZE 512

typedef enum { false, true } bool;

int checksum(char *);

void tarcreate(int , char *, bool , bool );
void tarextract(int , char *, bool , bool );
