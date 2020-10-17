/* file_ts.c - write the current date/time to a file
 * Usage: file_tm filename
 * Action: writes the current time/date to filename
 * Note: uses fcntl() - based locking
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>

#define oops(m,x) {perror(m); exit(x);}

void lock_operation(int, int);

int
main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "usage: file_ts filename\n");
        exit(1);
    }

    int fd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if( fd == -1 )  oops(argv[1], 2);
    
    time_t now;
    char* message;
    printf("Storing time in %s...\n", argv[1]);
    while(1) {
        time(&now);
        message = ctime(&now);          // compute time

        lock_operation(fd, F_WRLCK);    // lock for writing

        if( lseek(fd, 0L, SEEK_SET) == -1 )
            oops("lseek", 3);
        if( write(fd, message, strlen(message)) == -1 )
            oops("write", 4);

        lock_operation(fd, F_UNLCK);    // unlock file
        sleep(1);   // wait for new time
    }
    return 0;
}

void lock_operation(int fd, int op) {
    struct flock lock;

    lock.l_whence = SEEK_SET;
    lock.l_start = lock.l_len = 0;
    lock.l_pid = getpid();
    lock.l_type = op;

    if( fcntl(fd, F_SETLKW, &lock) == -1 )
        oops("lock operation", 6)
}
