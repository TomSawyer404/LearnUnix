/* who03.c - version 3 of the who program
 * - open, read UTMP file, and show result
 * - suppresses empty records
 * - formats time nicely
 * - buffers input (using utmplib)
 **/

#include <stdio.h>
#include <stdlib.h>
#include "utmplib.h"
#include <fcntl.h>  // close()
#include <utmp.h>   // Read utmp file
#include <unistd.h> // read(), open()
#include <time.h>   // ctime()

void show_info(struct utmp*);

int
main(int argc, char** argv) {
    struct utmp* utbufp;  //holds pointer to next rec
    char utmp_path[20] = "/var/run/utmp";
    
    if( utmp_open(utmp_path) == -1 ) {
        perror(utmp_path);  // Failed to open a file
        exit(1);
    } 
    
    while( (utbufp = utmp_next()) != NULL )
        show_info(utbufp);

    utmp_close();
    return 0;
}


void show_info(struct utmp* current_record) {
    if(current_record->ut_type != 7)  // Display normal processes only
        return;

    time_t time = current_record->ut_time;
    char* strTime = (char*)malloc(20);
    strTime = ctime(&time);

    printf("%-8.8s ", current_record->ut_user);
    printf("%-8.8s ", current_record->ut_line);
    printf("%.20s ", strTime + 4);
    printf("(%-s)\n", current_record->ut_host);
}
