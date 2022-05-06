#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define BLOCKSIZE 512

typedef enum { false, true } bool;

int checksum(char *);
