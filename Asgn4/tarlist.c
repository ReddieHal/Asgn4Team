#include "helper.h"

char *filePerm(header *head) {
    int i;
    long int out;
    int modes[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    char bestCase[] = "rwxrwxrwx";
    char flag;
    char *perm = (char *)malloc(sizeof(char) * 10);

    flag = head->typeflag[0];
    switch (flag){
        case DIRECT:
            perm[0] = 'd';
            break;
        case SYMLINK:
            perm[0] = 'l';
            break;
        case REGFILE:
        case ALTFILE:
            perm[0] = '-';
            break;
        default:
            break;
    }

    for(i = 0; i < 9; i++) {
        out = strtol(head->mode, NULL, 8);
        if (out & modes[i]) {
            perm[i+1] = bestCase[i];
        } else {
            perm[i + 1] = '-';
        }
    }
    return perm;
}

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
           printf("%s\n", filePerm(head));
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