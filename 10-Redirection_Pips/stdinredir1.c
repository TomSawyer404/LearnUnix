/* stdinredir1.c
 * purpose: show how to redirect standard input by replacing file
 *   descriptor 0 with a connection to a file.
 * action: reads three 3lines from standard input, then
 *   close fd 0, open a disk file, then reads in three more lines 
 *   from standard input.
 **/

#include <stdio.h>
#include <fcntl.h>      // open()
#include <stdlib.h>     // exit()
#include <unistd.h>     // close()

int
main() {
    char line[100];
    
    /** read and print three lines */
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);

    /** redirect input */
    close(0);
    int fd = open("/etc/passwd", O_RDONLY);
    if( fd != 0) {
        fprintf(stderr, "Could not open data adn fd 0\n");
        exit(1);
    }

    /** read and print three lines */
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);
    fgets(line, 100, stdin);  printf("%s", line);

    return 0;
}
