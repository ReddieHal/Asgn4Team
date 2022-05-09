#include "helper.h"


int main(int argc, char *argv[]) {
    bool verbose = false, stdCmp = false;


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
            tarextract(1,NULL,verbose, stdCmp );
        }
        /* create tar */
        if ((strchr(argv[1],'c'))) {
            tarcreate(1,NULL,verbose, stdCmp );
        }
        /* list tar */
        if ((strchr(argv[1],'t'))) {
            
        }
        
        /*easter egg*/
        if (!(strchr(argv[1],'f'))) {
            perror("Does Not Handle STDIN/STDOUT");
            exit(1);
        }
    }
    return 0;
}