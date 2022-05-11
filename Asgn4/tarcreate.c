#include "helper.h"

#define MAX_NAME_LENGTH 256
#define BLOCK_SIZE 512
#define REG_FILE_TYPE 0
#define DIR_FILE_TYPE 5
#define LNK_FILE_TYPE 2

#define min(a, b) (((a) < (b)) ? (a) : (b))

void tarcreate(int file, char *path, bool verbose, bool strict);
int add_file_rec(int file, char *path);
char *create_header(char *path);
int copy_file(int dst, char *path);
int get_file_type(struct stat *inode);

void tarcreate(int file, char *path, bool verbose, bool strict) 
{
    printf("running tarcreate\n");
    if (add_file_rec(file, path) < 0)
    {
        perror("Create\n");
        exit(1);
    }
    close(file);
}

int add_file_rec(int file, char *path)
{
    /* get inode and file type */
    struct stat *inode;
    if (lstat(path, inode) < 0)
        return -1; /* error */
    int file_type;
    file_type = get_file_type(inode);
    if (file_type < 0)
        return -1;

    /* create and copy header */
    char *header;
    header = create_header(path);
    if (write(file, header, BLOCK_SIZE) < 0)
        return -1;
    free(header);

    /* base case: regular file or symbolic link */
    if (file_type == REG_FILE_TYPE || file_type == LNK_FILE_TYPE)
    {
        copy_file(file, path);
        return file;
    }

    /* directory */
    if (file_type == DIR_FILE_TYPE)
    {
        DIR *dir = opendir(path);
        readdir(dir); /* . */
        readdir(dir); /* .. */

        struct dirent *dir_entry;
        while((dir_entry = readdir(dir)) != NULL)
        {
            char *new_path = 
                (char *) malloc(strlen(path) + strlen(dir_entry->d_name) + 2);
            strcpy(new_path, path);
            new_path[strlen(path)] = '/';
            strcpy(new_path[strlen(path) + 1], dir_entry->d_name);
            new_path[strlen(path) + strlen(dir_entry->d_name) + 1] = '\0';
            if (add_file_rec(file, new_path) < 0)
                return -1;
        }
        return file;
    }

    return -1; /* error for all other file types */
}

char *create_header(char *path)
{
    /* get inode and file type */
    struct stat *inode;
    if (lstat(path, inode) < 0)
        return NULL; /* error */
    int file_type;
    if ((file_type = get_file_type(inode)) < 0)
        return NULL;

    /* allocate header */
    char *header;
    header = (char *) calloc(BLOCK_SIZE, 1);
    
    /* get file name */
    char *name = calloc(MAX_NAME_LENGTH, 1);
    int cur = strlen(path);
    while(path[cur - 1] != '/')
        cur--;
    strcpy(name, path[cur]);

    /* fill in header */
    snprintf(header[0], 100, name[min(0, strlen(name) - 100)]); /* name */
    snprintf(header[100], 8, "%o", inode->st_mode); /* mode */
    snprintf(header[108], 8, "%o", inode->st_uid); /* uid */
    snprintf(header[116], 8, "%o", inode->st_gid); /* gid */
    if (file_type == REG_FILE_TYPE) /* size if regular file */
        snprintf(header[124], 12, "%o", inode->st_size);
    else /* 0 if not regular file */
        snprintf(header[124], 12, "0");
    snprintf(header[136], 12, "%o", inode->st_mtime); /* mtime */
    /* checksum calculated later */
    snprintf(header[156], 1, "%d", file_type); /* file type */
    if (file_type == LNK_FILE_TYPE) /* link value if symlink */
        if (readlink(path, header + 157, 100) < 0)
            return NULL;
    snprintf(header[257], 6, "ustar"); /* "ustar" */
    snprintf(header[263], 2, "00"); /* "00" */
    if ((struct passwd *u_pwd = getpwuid(inode->st_uid)) != NULL)
        snprintf(header[265], 32, u_pwd->u_name);  /* user name */
    if ((struct group *gr_pwd = getgrgid(inode->st_gid)) != NULL)
        snprintf(header[297], 32, gr_pwd->gr_name); /* group name */
    snprintf(header[329], 8, "%o", major(inode->st_dev)); /* major number */
    snprintf(header[337], 8, "%o", minor(inode->st_dev)); /* minor number */
    snprintf(header[345], min(0, strlen(name) - 100), name[0]); /* prefix */

    /* checksum */
    int sum = 0;
    int i;
    for (i = 0; i < BLOCK_SIZE; i++)
        sum += (unsigned char) header[i];
    sum += 32 * 8; /* checksum width of 8 bytes and space is 32 in ascii */
    snprintf(header[148], 8, "%o", sum);

    free(name);
    return header;
}

int copy_file(int dst, char *path)
{
    if ((int src = open(path, O_RDONLY)) < 0)
        return -1;
    char *buf = (char *) calloc(BLOCK_SIZE, 1);
    int error_flag_r, error_flag_w;
    while ((error_flag_r = read(src, buf, BLOCK_SIZE)) > 0)
        if ((error_flag_w = write(dst, buf, BLOCK_SIZE)) < 0)
            break;
    free(buf);
    if (close(src) < 0)
        return -1;
    if (error_flag_r < 0 || error_flag_w < 0)
        return -1;
    return dst;
}

int get_file_type(struct stat *inode)
{
    if (S_ISREG(inode->st_mode))
        return REG_FILE_TYPE;
    else if (S_ISDIR(inode->st_mode))
        return DIR_FILE_TYPE;
    else if (S_ISLNK(inode->st_mode))
        return LNK_FILE_TYPE;
    else
        return -1;
}