/* ls1.c
 * pirpose lis contents of directory or directories
 * acion if no args, use . else list files in args
 * */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>  // readdir()

void do_ls(char*);

int
main(int argc, char** argv) {
    if(argc == 1)
        do_ls(".");
    else {
        while( --argc ) {
            printf("%s:\n", *++argv);
            do_ls(*argv);
        }
    }

    return 0;
}


void do_ls(char* dirName) {
    /** list files in directory called dirName */
    DIR* dirPointer;
    struct dirent* dirEntryPointer;

    if( (dirPointer = opendir(dirName)) != NULL ) {  // Open a directory successfully
        while( (dirEntryPointer = readdir(dirPointer)) != NULL ) {
            printf("%s\n", dirEntryPointer->d_name);
        }
        closedir(dirPointer);

    } else {  // Failed to open a directory
        perror(dirName);
    }
}
