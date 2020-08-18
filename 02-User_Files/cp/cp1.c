/* cp1 - version 1 of cp 
 * - uses read() and write() wih tunable buffer size
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // read()
#include <fcntl.h>   // creat()

#define COPYMODE 0644

int
main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: %s srcFile dstFile\n", *argv);
        exit(1);
    }

    int in_fd;
    if( (in_fd = open(argv[1], O_RDONLY)) == -1 ) {
        perror(argv[1]);
        exit(1);
    }

    int out_fd;
    if( (out_fd = creat(argv[2], COPYMODE)) == -1 ) {
        perror(argv[2]);
        exit(1);
    }

    int numOfChar;
    char buffer[2048];
    while( (numOfChar = read(in_fd, buffer, 2048)) > 0 ) {
        if( write(out_fd, buffer, numOfChar) != numOfChar )
            perror("Write Error");
    }
    if( numOfChar == -1 )  perror("Read Error");
    if( close(in_fd) == -1 || close(out_fd) == -1 )  perror("Error closing files");

    return 0;
}
