#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#define BLOCKSIZE 512
#define REGFILE '0'
#define ALTFILE '\0'
#define SYMLINK '2'
#define DIRECT '5'
#define BLANK "        "

typedef enum { false, true } bool;

int checksum(char *);

void tarcreate(int , char *, bool , bool );
void tarextract(int , char *, bool , bool );
