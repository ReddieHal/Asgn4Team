#include "helper.h"
#define SEGSIZE 4096
#define HEADSIZE 512

typedef struct header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
} header;


header* strToStruct(char *buff) {
    char *temp = buff;
    header *ret = (header *)malloc(sizeof(header));

    memcpy(&(ret->name),&temp[0], 100);
    memcpy(&(ret->mode),&temp[100], 8);
    memcpy(&(ret->uid),&temp[108], 8);
    memcpy(&(ret->gid),&temp[116], 8);
    memcpy(&(ret->size),&temp[124], 12);
    memcpy(&(ret->mtime),&temp[136], 12);
    memcpy(&(ret->chksum),&temp[148], 8);
    memcpy(&(ret->typeflag),&temp[156], 1);
    memcpy(&(ret->linkname),&temp[157], 100);
    memcpy(&(ret->magic),&temp[257], 6);
    memcpy(&(ret->version),&temp[263], 2);
    memcpy(&(ret->uname),&temp[265], 32);
    memcpy(&(ret->gname),&temp[297], 32);
    memcpy(&(ret->devmajor),&temp[329], 8);
    memcpy(&(ret->devminor),&temp[337], 8);
    memcpy(&(ret->prefix),&temp[345], 155);
    
    return ret;
}

void directCase(header *head) {
    char fullName[255];
    
    memcpy(fullName, head->prefix, 155);
    strcat(fullName, head->name);
    printf("%s\n", fullName);

    
    if (mkdir(fullName, S_IRWXU) < 0) {
        perror("mkdir");
        exit(1);
    }
}

void fileCase(header *head) {
    char fullName[255];
    
    memcpy(fullName, head->prefix, 155);
    strcat(fullName, head->name);
    printf("%s\n", fullName);

    
    if (creat(fullName, S_IRWXU) < 0) {
        perror("creat");
        exit(1);
    }
}

void tarextract(int file, char *path, bool verbose, bool strict) {
    char buf[SEGSIZE];
    char nu = '\0';
    char flag;
    header *head;
    int charCount, i, sum;
    long int out;
    /*read the header */
    printf("running tarextract\n");

    while((charCount = read(file, &buf, HEADSIZE)) > 0) {

    if (charCount < 0) {
        perror("read");
        exit(1);
    }
    
    /*unpack buffer string to header struct*/
    head = strToStruct(buf);
    if (strcmp(buf, &nu) == 0) {
        if ((read(file, &buf, HEADSIZE) > 0) && (strcmp(buf, &nu) == 0)) {
            printf("end of file\n");
            exit(0);
        }
    }
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
    } 

    flag = head->typeflag[0];
    
    switch(flag) {
        case DIRECT:
            directCase(head);
            break;
        case SYMLINK:
            break;
        default :
            printf("%s\n",head->name);
            out = strtol(head->size, NULL, 8);         
            if (out < 512 && out > 0) {
                out = 1;
            } else {
                out = out / BLOCKSIZE;
            }
            lseek(file, out * BLOCKSIZE, SEEK_CUR);
            fileCase(head);
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