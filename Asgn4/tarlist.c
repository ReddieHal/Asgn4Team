#include "helper.h"

/*properly parsing file permissions */
char *filePerm(header *head) {
    int i = 0;
    long int out = 0;
    int modes[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, \
    S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    char bestCase[] = "rwxrwxrwx";
    char flag;
    char *perm = (char *)calloc(10, sizeof(char));

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

/*formatting user and group name */
char *ugname(header *head) {
    char *out = (char *)calloc(65, sizeof(char));
    out[64] = '\0';
    if ((strlen(head->uname) + strlen(head->gname)) <= 64) {
        strcat(out, head->uname);
        strcat(out, "/");
        strcat(out, head->gname);
    } else {
        out = NULL;
    }

    return out;
}

void tarlist(int file, char **path,int pathsize, bool verbose, bool stdCmp) {
    char buf[SEGSIZE];
    char fullName[256];
    char timeBuf[18];
    char nu = '\0';
    char *flPermPtr, *ugnamePtr;
    int charCount, i, sum, diff;
    long int out;
    header *head;
    

    while((charCount = read(file, &buf, HEADSIZE)) > 0) {
        time_t *mtime = (time_t *)calloc(1, sizeof(time_t));
        struct tm *tinfo = NULL;

        memset(fullName, '\0', 256);
        memset(timeBuf, '\0', 18);

        if (charCount < 0) {
            perror("read");
            free(mtime);
            exit(1);
        }

        if (strcmp(buf, &nu) == 0) {
            if ((read(file, &buf, HEADSIZE) > 0) && \
            (strcmp(buf, &nu) == 0)) {
                free(mtime);
                exit(0);
            }
        }

        /*pass string read to struct */
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
            free(head);
            free(mtime);
            exit(1);
        }

        complianceChecker(head, stdCmp);

        bigName(head, fullName);
       
        /*nested cases
        * if not verbose and no path - printf
        * if not verbose and path - check match then printf
        * if verbose and no path - format and printf
        * if v and path - check match and printf 
        * then slide over/lseek size of file 
        */
        if (verbose == false) {
            if (path != NULL) {
                for (i = 0; i < pathsize; i++) {
                    if (strstr(fullName, path[i])) {
                        printf("%s\n", fullName);
                    }
                }
            } else {
                printf("%s\n", fullName);
            }
            free(mtime);
            
        } else {
           flPermPtr = filePerm(head);
           ugnamePtr = ugname(head);

           *mtime = (int)strtol(head->mtime, NULL, 8);
           tinfo = localtime(mtime);
           strftime(timeBuf, 17, "%Y-%m-%d %H:%M", tinfo);

            if (path != NULL) {
                for (i = 0; i < pathsize; i++) {
                    if (strstr(fullName, path[i])) {
                        printf("%s %-17s %8lu %s %s\n", flPermPtr, ugnamePtr, \
                        strtol(head->size, NULL, 8), \
                        timeBuf, fullName);
                    }
                }
            } else {
                printf("%s %-17s %8lu %s %s\n", flPermPtr, ugnamePtr, \
                strtol(head->size, NULL, 8), \
                timeBuf, fullName);
            }
            
           free(flPermPtr);
           free(ugnamePtr);
        
           free(mtime); 
        }
        

        out = strtol(head->size, NULL, 8);

        if ((out > 0) ) {
            diff = out / BLOCK_SIZE;
            if (diff > 0) {
                lseek(file, diff * BLOCK_SIZE, SEEK_CUR);
            }

            diff = (out - (diff * BLOCK_SIZE));
            if (diff > 0) {
                lseek(file, BLOCK_SIZE, SEEK_CUR);
            }
            
        }
        free(head);
    }
}