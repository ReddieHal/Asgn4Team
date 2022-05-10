#include "helper.h"
#define SEGSIZE 4096
#define HEADSIZE 512

void tarextract(int file, char *path, bool verbose, bool strict) {
    char buf[SEGSIZE];
    char checksum[8];
    char flag;
    int charCount, i, sum, out;
    /*read the header */
    printf("running tarextract\n");

    if ((charCount = read(file, &buf, HEADSIZE)) == -1) {
        perror("read");
        exit(1);
    }
    /*check check sum*/
    for (i = 0; i < 8; i++) {
        checksum[i] = buf[148 + i];
        buf[148 + i] = ' ';
    }
    
    sum = 0;
    for (i = 0; i < HEADSIZE; i++) {
        sum += (unsigned char)buf[i];
    }

    out = strtol(checksum, NULL, 8);

    if (out != sum) {
        fprintf(stderr, "Checksum doesn't match\n");
    } 

    flag = buf[156];

    switch(flag) {
        case DIRECT:
            printf("dire\n");
            break;
        case SYMLINK:
            break;
        default :
            break;
    }
    /*check the flag type*/
    /*if its a directory*/
    /*  make the directory */
    /*  rename the directory */
    /*  move to the directory */

    /*if its a file */
    /*  make/open a new file */
    /*  copy its contents */
    /* continue */
}