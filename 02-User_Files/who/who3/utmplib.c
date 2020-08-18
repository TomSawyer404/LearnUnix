/* utmplib.c - function to buffer reads from utmp file
 * 
 * function are
 *   utmp_open(filename)  - open file
 *      return -1 on error
 *   utmp_next()  - return poiner to next struct
 *      return NULL on eof
 *   utmp_close()  - close file
 *  
 *   reads UNIT per read and then doles them out from the buffer
 **/

#include "utmplib.h"
#include <stdio.h>
#include <fcntl.h>      // close()
#include <unistd.h>     // read(), open()
#include <utmp.h>       // Read UTMP file

#define UNIT 16         // Read 16-unit per system call()
#define UTSIZE (sizeof(struct utmp))

static char utmpbuf[ UNIT * UTSIZE ];
static int num_recs;        // num stored
static int cur_rec;         // next to go
static int fd_utmp = -1;    // read from


int 
utmp_open(char* filename) {
    fd_utmp = open(filename, O_RDONLY);
    cur_rec = num_recs = 0;
    return fd_utmp;
}


int
utmp_reload() {
    /** read next bunch of records into buffer */
    int temp = read(fd_utmp, utmpbuf, UNIT * UTSIZE);
    num_recs = temp / UTSIZE;
    cur_rec = 0;
    return num_recs;
}


struct utmp*
utmp_next() {
    struct utmp* temp;
    if( fd_utmp == -1 )  // error?
        return NULL;
    if( cur_rec == num_recs && utmp_reload() == 0 )  // any more?
        return NULL;
    
    temp = (struct utmp*)&utmpbuf[ cur_rec * UTSIZE ];
    cur_rec++;
    return temp;
}


void
utmp_close() {
    if( fd_utmp == -1 )
        close(fd_utmp);
}

