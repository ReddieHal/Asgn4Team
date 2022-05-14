#include "helper.h"
#define SEGSIZE 4096
#define HEADSIZE 512

void dirMaker(char *bigName) {
    char tempName[256];
    struct stat tempStat;
    int i, nameLen;

    memset(tempName, '\0', 255);
    nameLen = strlen(bigName);
    i = 0;

    while (i < nameLen) {
        if (bigName[i] == '/') {
            memcpy(tempName, bigName, i);

            if (lstat(tempName, &tempStat) == -1) {
                if (mkdir(tempName, S_IRWXU) < 0) {
                perror("mkdir");
                exit(1);
                }
            }
        }
        i++;
    }
}

void directCase(header *head, bool extract) {
    char fullName[256];
    memset(fullName, '\0', 255);
    bigName(head, fullName);

    if (extract == true) {
        dirMaker(fullName);
    }
    
}

void fileCase(header *head, int file, bool extract) {
    char fullName[256];
    char buf[BLOCKSIZE];
    int fd;
    long int out;
    memset(fullName, '\0', 255);
    bigName(head, fullName);

    if (extract == true) {
    
        dirMaker(fullName);

        if ((fd = open(fullName, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR)) < 0) {
        printf("%s", fullName);
        perror("fd-open");
        exit(1);
        }

        out = strtol(head->size, NULL, 8);

        while (out > BLOCKSIZE) {
            read(file, buf, BLOCKSIZE);
            write(fd, buf, BLOCKSIZE);
            out = out - BLOCKSIZE;
        }

        if(out > 0) {
            read(file, buf, out);
            write(fd, buf, out);
            out = BLOCKSIZE - out;
            lseek(file, out, SEEK_CUR);
        }
    } else {
        out = strtol(head->size, NULL, 8);

        while (out > BLOCKSIZE) {
            lseek(file, BLOCKSIZE, SEEK_CUR);
            out = out - BLOCKSIZE;
        }

        if(out > 0) {
            lseek(file, out, SEEK_CUR);
            out = BLOCKSIZE - out;
            lseek(file, out, SEEK_CUR);
        }
    }
}

void symCase(header *head, bool extract) {
    char fullName[256];
    memset(fullName, '\0', 255);
    
    if (extract == true) {
        bigName(head, fullName);

        dirMaker(fullName);

        if (symlink(head->linkname, fullName) < 0) {
            perror("link");
            exit(1);
        }
    }
    
}

void tarextract(int file, char **path,int pathsize, bool verbose, bool strict) {
    char buf[SEGSIZE];
    char fullName[256];
    char nu = '\0';
    char flag;
    bool cont = false;
    header *head;
    int charCount, i, sum;
    long int out;
    /*read the header */


    while((charCount = read(file, &buf, HEADSIZE)) > 0) {
        cont = false;
        memset(fullName, '\0', 255);
        if (charCount < 0) {
            perror("read");
            exit(1);
        }
        
        /*unpack buffer string to header struct*/
        if (strcmp(buf, &nu) == 0) {
            if ((read(file, &buf, HEADSIZE) > 0) && (strcmp(buf, &nu) == 0)) {
                exit(0);
            }
        }

        head = strToStruct(buf);

        /*check check sum*/
        /*clear blank in buf*/
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

        complianceChecker(head, strict);
        

        bigName(head, fullName);

        if (path != NULL) {
            for (i = 0; i < pathsize; i++) {
                if (strstr(fullName, path[i])) {
                    cont = true;
                }
            }
        } else {
            cont = true;
        }
        

        if (verbose == true) {
            printf("%s\n", fullName);
        }

        flag = head->typeflag[0];

        switch(flag) {
            case DIRECT:
                directCase(head, cont);
                break;
            case SYMLINK:
                symCase(head, cont);
                break;
            case REGFILE:
            case ALTFILE:
                fileCase(head, file, cont);
                break;
            default:
                fprintf(stderr, "Invalid File Type");
                break;
                
        }

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