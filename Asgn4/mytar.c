#include "helper.h"


int main(int argc, char *argv[]) {
    bool verbose = false, stdCmp = false;
    int file;
    char *temp = NULL;

    /* if args given > 1:
    *   - check if char exists in string
    *       - for "v" & "S" enable their usage
    *       - else follow their respective cmds 
    */
    if (argc > 1) {
        /* print per item */
        if ((strchr(argv[1],'v'))) {
            
            verbose = true;
        }
        /* strict compliance */
        if ((strchr(argv[1],'S'))) {
            stdCmp = true;
        }
        /* tar file */
        if ((strchr(argv[1],'f'))) {
            
        }

        /* extract tar */
        if ((strchr(argv[1],'x'))) {
            if (argc > 2) {
                if ((file = open(argv[2], O_RDONLY)) == -1) {
                    perror("open");
                    exit(1);
                }
            }
            if (argc > 3) {
                tarextract(file,argv[3],verbose, stdCmp );
            } else {
                tarextract(file,NULL,verbose, stdCmp );
            }
        }
        /* create tar */
        if ((strchr(argv[1],'c'))) {
            if (argc > 2) {
                if ((file = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, \
                    S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
                    perror("open");
                    exit(1);
                }
            }
            if (argc > 3) {
                if ((strcmp(argv[3], ".") == 0) || \
                (strcmp(argv[3], "..") == 0)) {
                    temp = getcwd(argv[3], PATHMAX);
                    tarcreate(file,temp,verbose, stdCmp );
                } else {
                    tarcreate(file,argv[3],verbose, stdCmp );
                }
                
            } else {
                tarcreate(file,NULL,verbose, stdCmp );
            }
        }
        /* list tar */
        if ((strchr(argv[1],'t'))) {
            if (argc > 2) {
                if ((file = open(argv[2], O_RDONLY)) == -1) {
                    perror("open");
                    exit(1);
                }
            }
            if (argc > 3) {
                tarlist(file,argv[3],verbose, stdCmp);
            } else {
                tarlist(file,NULL,verbose, stdCmp );
            }
           
        }
        
        /*easter egg*/
        if (!(strchr(argv[1],'f'))) {
            perror("Does Not Handle STDIN/STDOUT");
            exit(1);
        }
    }
    return 0;
}