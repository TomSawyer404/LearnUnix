/* spwd.c: a simplified version of pwd 
 *
 * starts in current directory and recursively
 * climbs up to root of filessystem, prints top part
 * then prints current part
 *
 * use readdir() to get info about each thing 
 *
 * bug: prints an empty string if run from "/"
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // strncpy()
#include <dirent.h>     // readdir()
#include <unistd.h>     // chdir(), struct stat
#include <sys/types.h>  // opendir()
#include <sys/stat.h>

ino_t getInodeNum(char*);
void PrintPathTo(ino_t);
void InumToName(ino_t, char*, int);


int
main(int argc, char** argv) {
    PrintPathTo( getInodeNum(".") );  // print path to here
    putchar('\n');  // then add newline
    return 0;
}


void PrintPathTo(ino_t this_inode) {
    /** prints path leading down to an object with this inode
     * kindof recursive
     **/
    ino_t my_inode;
    char its_name[128];
    if( getInodeNum("..") != this_inode ) {  // still a long way to root of tree
        chdir("..");
        InumToName(this_inode, its_name, 128);  // get its name

        my_inode = getInodeNum(".");
        PrintPathTo( my_inode );  // recursively
        printf("/%s", its_name);
    }
}


void InumToName(ino_t inode_to_find, char* nameBuf, int bufLen) {
    /** looks through current directory for a file with this inode
     * number and copies its name into namebuf
     **/
    DIR* dirPointer;
    struct dirent* direntPointer;
    if( (dirPointer = opendir(".")) == NULL ) {
        perror(".");
        exit(1);
    }
    
    /** search directory for a file with specified inum */
    while( (direntPointer = readdir(dirPointer)) != NULL ) {
        if(direntPointer->d_ino == inode_to_find) {
            strncpy(nameBuf, direntPointer->d_name, bufLen);
            nameBuf[bufLen - 1] = '\0';  // just in case
            closedir(dirPointer);
            return;
        }
    }

    fprintf(stderr, "error looking for inode-num: %ld\n", inode_to_find);
    exit(1);
}


ino_t getInodeNum(char* fileName) {
    /** returns inode-number of the file */
    struct stat info;
    if( stat(fileName, &info) == -1 ) {
        fprintf(stderr, "Cannot stat ");
        perror(fileName);
        exit(1);
    }
    return info.st_ino;
}
