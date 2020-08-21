/* ls2.c
 * pirpose lis contents of directory or directories
 * acion if no args, use . else list files in args
 * */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>     // readdir()
#include <sys/stat.h>   // stat info
#include <unistd.h>
#include <time.h>       // ctime()

void do_ls(char*);
void do_stat(char*);
void show_stat(char*, struct stat*);
void mode_to_string(int, char*);

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
            do_stat(dirEntryPointer->d_name);
        }
        closedir(dirPointer);

    } else {  // Failed to open a directory
        perror(dirName);
    }
}


void do_stat(char* fileName) {
    struct stat fileInfo;
    if( stat(fileName, &fileInfo) != -1 ) {
        show_stat(fileName, &fileInfo);
    } else {
        perror(fileName);
    }
}


void show_stat(char* fileName, struct stat* statPointer) {
    /** display some info from stat in a name = value format */   
    
    //char* timeStr = (char*)malloc(25);
    char* modeStr = (char*)malloc(20);

    //time_t time = statPointer->st_mtime;
    //timeStr = ctime(&time);
    mode_to_string(statPointer->st_mode, modeStr);

    printf("%s ", modeStr);
    printf("%-4ld", statPointer->st_nlink);
    printf("%-8d", statPointer->st_uid);
    printf("%-8d", statPointer->st_gid);
    printf("%-8ld", statPointer->st_size);
    printf("%.20s", 4 + ctime(&statPointer->st_mtime));
    printf(" %s\n", fileName);
}


void mode_to_string(int mode, char* modeStr) {
    strcpy(modeStr, "----------");  // deafult = no perms
    
    if( S_ISDIR(mode) )  modeStr[0] = 'd';  // directory?
    if( S_ISCHR(mode) )  modeStr[0] = 'c';  // char devices?
    if( S_ISBLK(mode) )  modeStr[0] = 'b';  // block device?

    if( mode & S_IRUSR )  modeStr[1] = 'r'; // 3 bits for user
    if( mode & S_IWUSR )  modeStr[2] = 'w';
    if( mode & S_IXUSR )  modeStr[3] = 'x';

    if( mode & S_IRGRP )  modeStr[4] = 'r'; // 3 bits for group
    if( mode & S_IWGRP )  modeStr[5] = 'w';
    if( mode & S_IXGRP )  modeStr[6] = 'x';

    if( mode & S_IROTH )  modeStr[7] = 'r'; // 3 bits for others
    if( mode & S_IWOTH )  modeStr[8] = 'w';
    if( mode & S_IXOTH )  modeStr[9] = 'x';
}
