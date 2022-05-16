#include "helper.h"

int main(int argc, char *argv[]) 
{
    bool verbose = false, stdCmp = false;
    int file, i, size;
    char *temp = NULL;
    char **paths;
    int flag_count;

    /* if args given > 1:
    *   - check if char exists in string
    *       - for "v" & "S" enable their usage
    *       - else follow their respective cmds 
    */
    if (argc == 1 || argc == 2) 
    {
        fprintf(stderr, "Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }

    /* count how many of 'c', 't', or 'x' are present */
    flag_count = 0;
    if (strchr(argv[1], 'c'))
        flag_count++;
    if (strchr(argv[1], 't'))
        flag_count++;
    if (strchr(argv[1], 'x'))
        flag_count++;
    if (flag_count != 1)
    {
        fprintf(stderr, "Usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
        exit(1);
    }

    /* print per item */
    if (strchr(argv[1],'v')) 
        verbose = true;

    /* strict compliance */
    if (strchr(argv[1],'S')) 
        stdCmp = true;

    /* easter egg */
    if (!strchr(argv[1],'f')) 
    {
        perror("Does Not Handle STDIN/STDOUT");
        exit(1);
    }

    /* create tar */
    if (strchr(argv[1],'c')) 
    {
        int i;
        if ((file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
            S_IRWXU | S_IRWXG | S_IRWXO)) == -1) 
        {
            fprintf(stderr, "File open error\n");
            perror(argv[2]);
            exit(1);
        }
        for (i = 3; i < argc; i++)
        {
            if (strcmp(argv[i], ".") == 0 || 
                strcmp(argv[i], "..") == 0) 
            {
                temp = getcwd(argv[i], MAX_PATH_LENGTH);
                tarcreate(file, temp, verbose);
            } 
            else 
                tarcreate(file, argv[i], verbose);
        }
        end(file);
    }

    /* list tar */
    if ((strchr(argv[1],'t'))) 
    {
        if (argc > 2) {
            if ((file = open(argv[2], O_RDONLY)) == -1) {
                perror("mytar: open");
                exit(1);
            }
        }
        if (argc > 3) {
            size = (argc - 3);
            paths = (char **)malloc(sizeof(char *) * size);
            for (i = 3; i < argc; i++) {
                paths[i - 3] = argv[i];
            }
            tarlist(file,paths,size, verbose, stdCmp);
        } else {
            tarlist(file,NULL, 0, verbose, stdCmp );
        }  
    }

    /* extract tar */
    if ((strchr(argv[1],'x'))) {
        if (argc > 2) {
            if ((file = open(argv[2], O_RDONLY)) == -1) {
                perror("mytar: open");
                exit(1);
            }
        }
        if (argc > 3) {
            size = (argc - 3);
            paths = (char **)malloc(sizeof(char *) * size);
            for (i = 3; i < argc; i++) {
                paths[i - 3] = argv[i];
            }
            tarextract(file,paths,size,verbose, stdCmp);
        } else {
            tarextract(file,NULL, 0, verbose, stdCmp );
        }
    }
    return 0;
}