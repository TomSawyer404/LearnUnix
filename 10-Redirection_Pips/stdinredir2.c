/* stdinredir2.c
 * shows two more methods for redirecting standard input
 * use #define to set one or the other
 **/

#include <stdio.h>
#include <fcntl.h>          // open()
#include <stdlib.h>         // exit()
#include <unistd.h>         // dup()

// #define CLOSE_DUP        // open, close, dup, close
// #define USE_DUP2         // open, dup2, close

int
main() {
    /** read and print three lines */
    char line[100];

    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);

    /** redirect input */
    int fd = open("./stdinredir2.c", O_RDONLY);        // open the disk file
    int newfd;

# ifdef CLOSE_DUP
    close(0);
    newfd = dup(fd);                        // copy open fd to 0
# else  
    newfd = dup2(fd, 0);                    // close 0, dup fd to 0
# endif

    if( newfd != 0 ) {
        fprintf(stderr, "Could not duplicate fd to 0\n");
        exit(1);
    }
    close(0);

    /** read and print three lines */
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);

    return 0;
}
