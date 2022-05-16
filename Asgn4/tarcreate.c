#include "helper.h"

#define MAX_ID 2097151
#define MAX_MODE 4095
#define BLOCK_SIZE 512
#define MAX_PATH_LENGTH 256
#define MAX_NAME_LENGTH 100
#define REG_FILE_TYPE 0
#define DIR_FILE_TYPE 5
#define LNK_FILE_TYPE 2
#define SPACE 32
#define CHECKSUM_WIDTH 8

void tarcreate(int file, char *path, bool verbose);
void end(int file);
int add_file_rec(int file, char *path, int offset, bool verbose);
int create_header(char *header, char *path, int offset, struct stat *inode, 
    int file_type);
int copy_file(int dst, char *path);
int get_file_type(struct stat *inode);
void fatal_error(char *str);
int insert_special_int(char *where, size_t size, int32_t val);

/* main create function */
void tarcreate(int file, char *path, bool verbose) 
{
    /* "The path that can be specified
    is not the Full Path" */
    int offset = strlen(path);
    while (path[offset - 1] != '/' && offset > 0)
        offset--;

    /* call recursive file archiver, all errors already handled */
    add_file_rec(file, path, offset, verbose);
}

/* adds null blocks and closes tar file */
void end(int file)
{
    /* allocate empty block */
    char *empty = (char *) calloc(BLOCK_SIZE, 1);
    if (empty == NULL)
        fatal_error("Memory error\n");

    /* print 2 empty blocks */
    if (write(file, empty, BLOCK_SIZE) < 0)
        fatal_error("File write error\n");
    if (write(file, empty, BLOCK_SIZE) < 0)
        fatal_error("File write error\n");
    
    /* free empty block */
    free(empty);
    
    /* close */
    if (close(file) < 0)
        fatal_error("File close error\n");
}

/* recursive file archiver */
int add_file_rec(int file, char *path, int offset, bool verbose)
{
    /* declare and get stat */
    struct stat *inode = (struct stat *) malloc(sizeof(struct stat));
    if (inode == NULL)
    {
        fprintf(stderr, "Memory error\n");
        return -1;
    }
    if (lstat(path, inode) < 0)
    {
        fprintf(stderr, "Can't get file info: %s\n", path);
        free(inode);
        return -1;
    }

    /* get file type */
    int file_type = get_file_type(inode);
    if (file_type < 0)
    {
        fprintf(stderr, "Unsupported file type: %s\n", path);
        free(inode);
        return -1;
    }

    /* create and copy header */
    char *header = (char *) calloc(BLOCK_SIZE, 1);
    if (header == NULL)
    {
        fprintf(stderr, "Memory error\n");
        free(inode);
        return -1;
    }
    if (create_header(header, path, offset, inode, file_type) < 0)
    {
        free(header);
        free(inode);
        return -1;
    }
    if (write(file, header, BLOCK_SIZE) < 0)
    {
        fprintf(stderr, "Can't write header: %s\n", path);
        free(header);
        free(inode);
        return -1;
    }
    free(header);

    /* verbose mode */
    if (verbose)
        printf("%s\n", path);

    /* base case: regular file or symbolic link */
    if (file_type == REG_FILE_TYPE || file_type == LNK_FILE_TYPE)
    {
        if (copy_file(file, path) < 0)
            fprintf(stderr, "Can't copy file: %s", path);
        free(inode);
        return file;
    }

    /* directory */
    if (file_type == DIR_FILE_TYPE)
    {
        /* open directory and read . and .. */
        DIR *dir = opendir(path);
        if (dir == NULL)
        {
            fprintf(stderr, "Can't open directory: %s\n", path);
            free(inode);
            return -1;
        }
        if (readdir(dir) == NULL) /* . */
        {
            fprintf(stderr, "Can't read directory: %s\n", path);
            free(inode);
            return -1;
        }
        if (readdir(dir) == NULL) /* .. */
        {
            fprintf(stderr, "Can't read directory: %s\n", path);
            free(inode);
            return -1;
        }

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

            /* adds new path, all errors already handled */
            add_file_rec(file, new_path, offset, verbose);

            /* frees new path */
            free(new_path);

            /* reads next directory entry */
            dir_entry = readdir(dir);
        }

        free(inode);
        /* closes directory and returns */
        if (closedir(dir) < 0)
        {
            fprintf(stderr, "Can't close directory: %s\n", path);
            return -1;
        }
        return file;
    }

    /* should never reach this code */
    free(inode);
    return -1;
}

/* creates header from file path, puts header in buffer */
int create_header(char *header, char *path, int offset, struct stat *inode, 
    int file_type)
{
    /* add final slash if path is a directory */
    char *new_path = (char *) calloc(strlen(path) + 2, 1);
    strcpy(new_path, path);
    if (file_type == DIR_FILE_TYPE)
        new_path[strlen(path)] = '/';

    /* error if path is too long */
    if (strlen(new_path + offset) > MAX_PATH_LENGTH)
    {
        fprintf(stderr, "Path too long: %s\n", path);
        free(new_path);
        return -1;
    }
    
    /* allocate and copy name */
    char *name = calloc(MAX_NAME_LENGTH + 1, 1);
    if (name == NULL)
    {
        fprintf(stderr, "Memory error\n");
        free(new_path);
        return -1;
    }
    int cur;
    if (strlen(new_path + offset) > 100)
    {
        cur = strlen(new_path + offset) - 101; /*101 since char 0 may be '/' */
        while (path[cur] != '/' && cur < strlen(new_path + offset))
            cur++;
        if (cur >= strlen(new_path + offset))
        {
            fprintf(stderr, "File name too long: %s\n", path);
            free(new_path);
            free(name);
            return -1;
        }
        cur++;
    }
    else
        cur = 0;
    strcpy(name, new_path + offset + cur);

    /* name */
    snprintf(header + 0, 101, name);

    /* mode */
    snprintf(header + 100, 8, "%.7o", inode->st_mode & MAX_MODE);

    /* user ID */
    if (inode->st_uid <= MAX_ID)
        snprintf(header + 108, 8, "%.7o", inode->st_uid);
    else
        insert_special_int(header + 108, 8, inode->st_uid);

    /* group ID */
    if (inode->st_gid <= MAX_ID)
        snprintf(header + 116, 8, "%.7o", inode->st_gid);
    else
        insert_special_int(header + 108, 8, inode->st_gid);

    /* size */
    if (file_type == REG_FILE_TYPE)
        snprintf(header + 124, 12, "%.11o", (unsigned int) inode->st_size);
    else
        snprintf(header + 124, 12, "%.11o", 0);

    /* mtime */
    snprintf(header + 136, 12, "%.11o", (unsigned int) inode->st_mtim.tv_sec);

    /* file type */
    snprintf(header + 156, 2, "%d", file_type);

    /* link value if symlink */
    if (file_type == LNK_FILE_TYPE)
        if (readlink(path, header + 157, 100) < 0)
        {
            fprintf(stderr, "Symbolic link reading error: %s\n", path);
            free(new_path);
            free(name);
            return -1;
        }

    /* "ustar" */
    snprintf(header + 257, 6, "ustar");

    /* "00" */
    snprintf(header + 263, 3, "00");

    /* user name */
    struct passwd *u_pwd = getpwuid(inode->st_uid);
    if (u_pwd == NULL)
    {
        fprintf(stderr, "User name error: %s\n", path);
        free(new_path);
        free(name);
        return -1;
    }
    snprintf(header + 265, 31, u_pwd->pw_name);

    /* group name */
    struct group *gr_pwd = getgrgid(inode->st_gid);
    if (gr_pwd == NULL)
    {
        fprintf(stderr, "Group name error: %s\n", path);
        free(new_path);
        free(name);
        return -1;
    }
    snprintf(header + 297, 31, gr_pwd->gr_name);

    /* major and minor number not needed, doesn't support special files */

    /* prefix */
    snprintf(header + 345, 
        strlen(new_path + offset) - strlen(name), new_path + offset);

    /* checksum */
    int sum = 0;
    int i;
    for (i = 0; i < BLOCK_SIZE; i++)
        sum += (unsigned char) header[i];
    sum += CHECKSUM_WIDTH * SPACE;
    snprintf(header + 148, 8, "%.7o", sum);

    /* free memory and return */
    free(new_path);
    free(name);
    return 0;
}

/* copies file */
int copy_file(int dst, char *path)
{
    /* open file */
    int src = open(path, O_RDONLY);
    if (src < 0)
    {
        fprintf(stderr, "File open error: %s\n", path);
        return -1;
    }

    /* allocate buffer */
    char *buf = (char *) calloc(BLOCK_SIZE, 1);
    if (buf == NULL)
        fprintf(stderr, "Memory error\n");
    
    int error_flag_r;
    int error_flag_w;
    while (1)
    {
        /* read */
        error_flag_r = read(src, buf, BLOCK_SIZE);
        if (error_flag_r < 0)
            fprintf(stderr, "File read error: %s\n", path);
        if (error_flag_r <= 0)
            break;

        /* write */
        error_flag_w = write(dst, buf, BLOCK_SIZE);
        if (error_flag_w < 0)
        {
            fprintf(stderr, "File write error: %s\n", path);
            break;
        }
    }
    free(buf);
    if (close(src) < 0)
    {
        fprintf(stderr, "File close error: %s\n", path);
        return -1;
    }
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
void fatal_error(char *str)
{
    fprintf(stderr, str);
    exit(1);
}

/* for handling id over 7 octal digits */
int insert_special_int(char *where, size_t size, int32_t val) 
{
    /* For interoperability with GNU tar. GNU seems to
    * set the high–order bit of the first byte, then
    * treat the rest of the field as a binary integer
    * in network byte order.
    * Insert the given integer into the given field
    * using this technique. Returns 0 on success, nonzero
    * otherwise
    */
    int err = 0;
    if (val < 0 || (size < sizeof(val))) 
    {
        /* if it’s negative, bit 31 is set and we can’t use the flag
        * if len is too small, we can’t write it. Either way, we’re
        * done.
        */
        err++;
    } 
    else 
    {
        /* game on....*/
        memset(where, 0, size); /* Clear out the buffer */
        *(int32_t *)(where+size-sizeof(val)) = htonl(val); /* place the int */
        *where |= 0x80; /* set that high–order bit */
    }
    return err;
}