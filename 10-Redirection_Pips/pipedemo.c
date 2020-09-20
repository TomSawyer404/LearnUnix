/* pipedemo.c 
 * Demonstrates: how to create and use a pipe
 * Effect: creates a pipe, writes into writing end,
 *          then runs around and reads from reading
 *          end. A little weird, but demonstrates 
 *          the idea.
 **/

#include <stdio.h>
#include <unistd.h>     // pipe(), write()
#include <string.h>     // strlen()

int
main() {
    int aPipe[2];

    /** get a pipe */
    if( pipe( aPipe ) == -1 ) {
        perror("could not make pipe");
        return 1;
    }
    printf("Got a pipe! It is file descriptors: {%d, %d}\n", 
            aPipe[0], aPipe[1]);

    /** read from stdin, write into pipe, read from pipe, print */
    char buf[BUFSIZ];
    int len;
    while( fgets(buf, BUFSIZ, stdin) ) {
        len = strlen( buf );
        if( write(aPipe[1], buf, len) != len ) {    // send
            perror("writing to pipe");              // down
            break;                                  // pipe
        }
        for(int i = 0; i < len; i++)                // wipe
            buf[i] = 'X';
        len = read(aPipe[0], buf, BUFSIZ);          // read
        if( len == -1 ) {                           // from
            perror("reading from pipe");            // pipe
            break;
        }
        if( write(1, buf, len) != len ) {           // send
            perror("writing to stdout");            // to
            break;                                  // pipe
        }
    }
    
    return 0;
}
