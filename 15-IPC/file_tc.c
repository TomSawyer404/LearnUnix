/* file_tc.c - read the current date/time from a file
 * Usage: file_tc filename
 * Uses: fcntl() - based locking
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>

#define oops(m,x) {perror(m); exit(x);}
#define BUFLEN 10

void lock_operation(int fd, int op); 


int 
main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "usage: file_tc filename\n");
        exit(1);
    }
    
    int fd = open(argv[1], O_RDONLY);
    if( fd == -1 )
        oops(argv[1], 3);

    lock_operation(fd, F_RDLCK);
    
    int nread;
    char buf[BUFLEN];
    while( (nread = read(fd, buf, BUFLEN)) > 0 ) {
        write(1, buf, nread);  // write to stdout
    }

    lock_operation(fd, F_UNLCK);
    close(fd);
    return 0;
}


void lock_operation(int fd, int op) {
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_start = lock.l_len = 0;
    lock.l_pid = getpid();
    lock.l_type = op;

    if( fcntl(fd, F_SETLKW, &lock) == -1 )
        oops("lock operation", 6);
}
