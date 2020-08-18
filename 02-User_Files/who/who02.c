/* who02.c - version 2 of the who program
 * - open, read UTMP file, and show result
 * - suppresses empty records
 * - formats time nicely
 **/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  // close()
#include <utmp.h>   // Read utmp file
#include <unistd.h> // read(), open()
#include <time.h>   // ctime()

void show_info(struct utmp*);

int
main(int argc, char** argv) {
    struct utmp current_record;
    char utmp_path[20] = "/var/run/utmp";
    int utmp_fd;
    
    if( (utmp_fd = open(utmp_path, O_RDONLY)) == -1 ) {
        perror(utmp_path);  // Failed to open a file
        exit(1);
    } 
    
    int sizeUTMP = sizeof(current_record);
    while( read(utmp_fd, &current_record, sizeUTMP) == sizeUTMP )
        show_info(&current_record);

    close(utmp_fd);
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
