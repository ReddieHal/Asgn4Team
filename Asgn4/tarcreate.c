/*
Further tasks:

Implement verbose mode.
Write header the same way as Nico's demo.
Fully comply with assignment specs in error handling.
Debug.
*/

#include "helper.h"

#define BLOCK_SIZE 512
#define MAX_PATH_LENGTH 256
#define MAX_NAME_LENGTH 100
#define REG_FILE_TYPE 0
#define DIR_FILE_TYPE 5
#define LNK_FILE_TYPE 2
#define SPACE 32
#define CHECKSUM_WIDTH 8

#define min(a, b) (((a) < (b)) ? (a) : (b))

void tarcreate(int file, char *path, bool verbose, bool strict);
int add_file_rec(int file, char *path);
char *create_header(char *path);
int copy_file(int dst, char *path);
int get_file_type(struct stat *inode);
void error(char *str);

/* main create function */
void tarcreate(int file, char *path, bool verbose, bool strict) 
{
    /* call recursive file archiver */
    if (add_file_rec(file, path) < 0)
        error("create\n");

    /* allocate empty block */
    char *empty = (char *) calloc(BLOCK_SIZE, 1);
    if (empty == NULL)
        error("memory allocation\n");

    /* print 2 empty blocks */
    if (write(file, empty, BLOCK_SIZE) < 0)
        error("write\n");
    if (write(file, empty, BLOCK_SIZE) < 0)
        error("write\n");
    
    /* free empty block*/
    free(empty);
    
    /* close */
    if (close(file) < 0)
        error("close\n");
}

/* recursive file archiver */
int add_file_rec(int file, char *path)
{
    /* declare and get stat */
    struct stat *inode = (struct stat *) malloc(sizeof(struct stat));
    if (inode == NULL)
        return -1;
    if (lstat(path, inode) < 0)
        return -1;

    /* get file type */
    int file_type = get_file_type(inode);
    if (file_type < 0)
        return -1;

    /* create and copy header */
    char *header = create_header(path);
    if (header == NULL)
        return -1;
    if (write(file, header, BLOCK_SIZE) < 0)
        return -1;
    free(header);

    /* base case: regular file or symbolic link */
    if (file_type == REG_FILE_TYPE || file_type == LNK_FILE_TYPE)
    {
        if (copy_file(file, path) < 0)
            perror("Corrupt file\n");
        return file;
    }

    /* directory */
    if (file_type == DIR_FILE_TYPE)
    {
        /* open directory and read . and .. */
        DIR *dir = opendir(path);
        if (dir == NULL)
            return -1;
        if (readdir(dir) < 0) /* . */
            return -1;
        if (readdir(dir) < 0) /* .. */
            return -1;

        /* recursively reads all dir entries */
        struct dirent *dir_entry = readdir(dir);
        while(dir_entry != NULL)
        {
            /* allocates new path name, old name + '/' + new name + '\0' */
            char *new_path = 
                (char *) malloc(strlen(path) + strlen(dir_entry->d_name) + 2);
            
            /* copies old path to new path */
            strcpy(new_path, path);

            /* adds slash */
            new_path[strlen(path)] = '/';

            /* copies name to new path */
            strcpy(new_path + strlen(path) + 1, dir_entry->d_name);

            /* adds null terminator */
            new_path[strlen(path) + strlen(dir_entry->d_name) + 1] = '\0';

            /* adds new path */
            if (add_file_rec(file, new_path) < 0)
                perror("Corrupt file\n");

            /* frees new path */
            free(new_path);

            /* reads next directory entry */
            dir_entry = readdir(dir);
        }

        /* closes directory and returns */
        if (closedir(dir) < 0)
            return -1;
        return file;
    }
    free(inode);

    /* error for all other file types */
    return -1;
}

/* creates and returns header from file path */
char *create_header(char *path)
{
    /* error if path is too long */
    if (strlen(path) > MAX_PATH_LENGTH)
        return NULL;
    
    /* get inode and file type */
    struct stat *inode = (struct stat *) malloc(sizeof(struct stat));
    if (inode == NULL)
        return NULL;
    if (lstat(path, inode) < 0)
        return NULL;
    int file_type = get_file_type(inode);
    if (file_type < 0)
        return NULL;

    /* allocate header */
    char *header = (char *) calloc(BLOCK_SIZE, 1);
    if (header == NULL)
        return NULL;
    
    /* allocate and copy name */
    char *name = calloc(MAX_NAME_LENGTH + 1, 1);
    if (name == NULL)
        return NULL;
    int cur = min(1, strlen(path) - 100); /* char 0 is slash */
    while(path[cur - 1] != '/' && cur < strlen(path))
        cur++;
    if (cur >= strlen(path))
        return NULL;
    strcpy(name, path + cur);

    /* fill in header */
    snprintf(header + 0, 100, name); /* name */
    snprintf(header + 100, 8, "%.7o", inode->st_mode); /* mode */
    snprintf(header + 108, 8, "%.7o", inode->st_uid); /* uid */
    snprintf(header + 116, 8, "%.7o", inode->st_gid); /* gid */
    if (file_type == REG_FILE_TYPE) /* size if regular file */
        snprintf(header + 124, 12, "%.11o", (unsigned int) inode->st_size);
    else /* 0 if not regular file */
        snprintf(header + 124, 12, "%.11o", 0);
    snprintf(header + 136, 12, "%.11o", 
        (unsigned int) inode->st_mtime); /* mtime */
    /* checksum calculated later */
    snprintf(header + 156, 1, "%d", file_type); /* file type */
    if (file_type == LNK_FILE_TYPE) /* link value if symlink */
        if (readlink(path, header + 157, 100) < 0)
            return NULL;
    snprintf(header + 257, 6, "ustar"); /* "ustar" */
    snprintf(header + 263, 2, "00"); /* "00" */
    struct passwd *u_pwd = getpwuid(inode->st_uid);
    if (u_pwd == NULL)
        return NULL;
    snprintf(header + 265, 31, u_pwd->pw_name);  /* user name */
    struct group *gr_pwd = getgrgid(inode->st_gid);
    if (gr_pwd == NULL)
        return NULL;
    snprintf(header + 297, 31, gr_pwd->gr_name); /* group name */
    snprintf(header + 329, 8, "%.7o", major(inode->st_dev)); /* major number */
    snprintf(header + 337, 8, "%.7o", minor(inode->st_dev)); /* minor number */
    snprintf(header + 345, strlen(path) - strlen(name) - 1, path); /* prefix */

    /* checksum */
    int sum = 0;
    int i;
    for (i = 0; i < BLOCK_SIZE; i++)
        sum += (unsigned char) header[i];
    sum += CHECKSUM_WIDTH * SPACE;
    snprintf(header + 148, 8, "%o", sum);

    /* free memory and return */
    free(name);
    free(inode);
    return header;
}

/* copies file */
int copy_file(int dst, char *path)
{
    /* open file */
    int src = open(path, O_RDONLY);
    if (src < 0)
        return -1;

    /* allocate buffer */
    char *buf = (char *) calloc(BLOCK_SIZE, 1);
    
    int error_flag_r;
    int error_flag_w;
    while (1)
    {
        /* read */
        error_flag_r = read(src, buf, BLOCK_SIZE);
        if (error_flag_r < 0)
            return -1;
        if (error_flag_r == 0)
            break;

        /* write */
        error_flag_w = write(dst, buf, BLOCK_SIZE);
        if (error_flag_w < 0)
            return -1;
    }
    free(buf);
    if (close(src) < 0)
        return -1;
    return dst;
}

/* get file type */
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

/* prints error and exits */
void error(char *str)
{
    perror(str);
    exit(1);
}