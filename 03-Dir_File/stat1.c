/* stat1.c - version 1 of stat
 * - use stat() system call to obtain and print file properties.
 * - some members are just numbers...
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // stat()
#include <sys/stat.h>
#include <time.h>    // ctime()

void show_stat(char*, struct stat*);

int
main(int argc, char** argv) {
    struct stat fileInfo;

    if(argc > 1) {
        if( stat(argv[1], &fileInfo) != -1) {
            show_stat(argv[1], &fileInfo);
            return 0;
        } else {
            perror(argv[1]);
        }
    }
    return 1;
}


void show_stat(char* fileName, struct stat* statPointer) {
    /** display some info from stat in a name = value format */

    time_t time = statPointer->st_mtime;
    char* strTime = (char*)malloc(25);
    strTime = ctime(&time);

    printf("mode: %o\n", statPointer->st_mode);
    printf("links: %ld\n", statPointer->st_nlink);
    printf("user: %d\n", statPointer->st_uid);
    printf("group: %d\n", statPointer->st_gid);
    printf("size: %ld\n", statPointer->st_size);
    printf("name: %s\n", fileName);
    printf("modified_time: %.25s\n", strTime + 4);
}
