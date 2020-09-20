/* pipedemo2.c
 * Demonstrates how pipe is duplicated in fork()
 * Parent continues to write and read pipe,
 * but child also writes to the pipe
 **/

#include <stdio.h>
#include <string.h>     // strlen()
#include <unistd.h>     // fork(), pipe(), sleep()

#define CHILD_MESS  "I wan a cookie\n"
#define PAR_MESS    "Testing...\n"
#define oops(m,x)   { perror(m); return x; }

int
main() {
    int pipeFd[2];
    if( pipe( pipeFd ) == -1 )  oops("cannot get a pipe", 1);
    
    char buf[BUFSIZ];
    int len = 0, readlen = 0;
    switch( fork() ) {
        case -1:
            oops("cannot fork", 2);

        /** child writes to pipe every 5 seconds */
        case 0:
            while(1) {
                len = strlen(CHILD_MESS);
                if( write(pipeFd[1], CHILD_MESS, len) != len )
                    oops("wirte", 3);
                sleep(5);
            }

        /** parent reads from pipe and also writes to pipe */
        default:
            len = strlen(PAR_MESS);
            while(1) {
                if( write(pipeFd[1], PAR_MESS, len) != len ) 
                    oops("write", 3);
                sleep(1);

                readlen = read(pipeFd[0], buf, BUFSIZ);
                if( readlen < 0 ) 
                    break;
                write(1, buf, readlen);     // write into stdout
            }
    }

    return 0;
}
