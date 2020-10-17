/* selectdemo.c: watch for input on two devices AND timeout
 * usage: selectdemo dev1 dev2 timeout
 * action: reports on input from each file, and reports timeours
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define oops(m,x) {perror(m); exit(x);}

void showdata(char* fname, int fd); 


int
main(int argc, char** argv) {
    if(argc != 4) {
        fprintf(stderr, "usage: %s file file timeout", *argv);
        exit(1);
    }

    /* Open files */
    int fd1, fd2;   // the fds to watch
    if( (fd1 = open(argv[1], O_RDONLY)) == -1 )
        oops(argv[1], 2);
    if( (fd2 = open(argv[2], O_RDONLY)) == -1 )
        oops(argv[2], 3);
    int maxfd = 1 + (fd1 > fd2 ? fd1: fd2);   // max fd plus 1
    
    fd_set readfds;             // watch these for input
    int retval;                 // return from select
    struct timeval timeout;     // how long to wait
    while(1) {
        /* make a list of file descriptors to watch */
        FD_ZERO(&readfds);          // clear all bits
        FD_SET(fd1, &readfds);      // set bit for fd1
        FD_SET(fd1, &readfds);      // set bit for fd2

        /* set timeout value */
        timeout.tv_sec = atoi(argv[3]);     // set seconds
        timeout.tv_usec = 0;                // no useconds

        /* wait for input */
        retval = select(maxfd, &readfds, NULL, NULL, &timeout);
        if( retval == -1 )
            oops("select", 4);
        if( retval > 0 ) {
            /* check bits for reach fd */
            if( FD_ISSET(fd1, &readfds) )
                showdata(argv[1], fd1);
            if( FD_ISSET(fd2, &readfds) )
                showdata(argv[2], fd2);
        } else {
            printf("no input after %d seconds\n", atoi(argv[3]));
        }
    }

    return 0;
}


void showdata(char* fname, int fd) {
    char buf[BUFSIZ];
    int n;

    printf("%s: ", fname);
    fflush(stdout);
    
    n = read(fd, buf, BUFSIZ);
    if( n == -1 )
        oops(fname, 5);
    write(1, buf, n);
    write(1, "\n", 1);
}

