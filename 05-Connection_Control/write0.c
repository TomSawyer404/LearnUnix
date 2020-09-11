/* write0.c
 *
 * purpuse: send messages to anothoer terminal
 * method: open the other terminal for output then
 *          copy from stdin to that terminal 
 * shows: a terminal is just a file supporting regular i/o
 * usage: write0 ttyname
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // write()
#include <fcntl.h>   // open()

int
main(int argc, char** argv) {
    if(argc != 2) {  // check arguments
        fprintf(stderr, "usage: write0 ttyname\n");
        exit(1);
    }

    /** open devices */
    int fileDescriptor = open(argv[1], O_WRONLY);
    if( fileDescriptor == -1 ) {
        perror(argv[1]);
        exit(1);
    }

    /** loop until EOF on input*/
    char buffer[2048];
    while( fgets(buffer, 2048, stdin) != NULL ) {
        if( write(fileDescriptor, buffer, strlen(buffer)) == -1 )
            break;
    }
    close(fileDescriptor);

    return 0;
}
