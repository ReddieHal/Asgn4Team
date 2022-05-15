#include "helper.h"

int checksum(char *blockHead) {
    
    int i, value = 0;

    for (i = 0; i < BLOCKSIZE; i++) {
        value += (unsigned char)blockHead[i];
    }

    return value;
    
}

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

void bigName(header* head, char *fullName) {
    memcpy(fullName, head->prefix, 155);

    if (strlen(head->prefix) > 155) {
        memcpy(&fullName[155], "/", 1);
    } else {
        if (strlen(head->prefix) > 0) {
            memcpy(&fullName[strlen(head->prefix)], "/", 1);
        } 
    }

    if (strlen(head->prefix) > 0) {
        memcpy(&fullName[strlen(head->prefix) + 1], head->name, 100);
    } else {
        memcpy(&fullName[strlen(head->prefix)], head->name, 100);
    }
}

void complianceChecker(header* head, bool comp) {
    char magic[] = "ustar";
    char doubleOO[] = "00";

    if (comp == false) {
        if (memcmp(head->magic, magic, 5)) {
            fprintf(stderr,"Compliance Failed: Magic String");
            exit(1);
        }
    } else {
        if (memcmp(head->magic, magic, 5)) {
            fprintf(stderr,"Compliance Failed: Magic String");
            exit(1);
        }
        if (memcmp(head->version, doubleOO, 2)) {
            fprintf(stderr,"Compliance Failed: Version");
            exit(1);
        }
    }

}