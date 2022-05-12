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
#include <pwd.h>
#include <grp.h>

#define SEGSIZE 4096
#define PATHMAX 256
#define HEADSIZE 512
#define BLOCKSIZE 512
#define REGFILE '0'
#define ALTFILE '\0'
#define SYMLINK '2'
#define DIRECT '5'
#define BLANK "        "

typedef enum { false, true } bool;

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

int checksum(char *);

void tarcreate(int , char *, bool , bool );
void tarextract(int , char *, bool , bool );
void tarlist(int , char *, bool , bool );

header* strToStruct(char *);
