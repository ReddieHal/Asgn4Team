#include "helper.h"

void tarlist(int file, char *path, bool verbose, bool stdCmp) {
    char buf[SEGSIZE];
    char fullName[255];
    char nu = '\0';
    int charCount, i, sum, diff;
    long int out;
    header *head;

    while((charCount = read(file, &buf, HEADSIZE)) > 0) {
        if (charCount < 0) {
            perror("read");
            exit(1);
        }

        if (strcmp(buf, &nu) == 0) {
            if ((read(file, &buf, HEADSIZE) > 0) && (strcmp(buf, &nu) == 0)) {
                exit(0);
            }
        }

        head = strToStruct(buf);

        memcpy(&buf[148], BLANK, 8);

        /*add up num*/
        sum = 0;
        for (i = 0; i < HEADSIZE; i++) {
            sum += (unsigned char)buf[i];
        }

        /*string to usable num*/
        out = strtol(head->chksum, NULL, 8);

        if (out != sum) {
            fprintf(stderr, "Checksum doesn't match\n");
            exit(1);
        }

        if (verbose == false) {
            memcpy(fullName, head->prefix, 155);
            strcat(fullName, head->name);
            printf("%s\n", fullName);
        } else {
           /* printf("%10s%17s%8o%16s"); */
        }
        

        out = strtol(head->size, NULL, 8);

        if ((out > 0) ) {
            diff = out / BLOCKSIZE;
            if (diff > 0) {
                lseek(file, diff * BLOCKSIZE, SEEK_CUR);
            }

            diff = (out - (diff * BLOCKSIZE));
            if (diff > 0) {
                lseek(file, BLOCKSIZE, SEEK_CUR);
            }
            
        }
    }


}