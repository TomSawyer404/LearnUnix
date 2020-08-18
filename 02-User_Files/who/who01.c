/* who01.c - version 1 of the who program
 * - open, read UTMP file, and show result
 **/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  // close()
#include <utmp.h>   // Read utmp file
#include <unistd.h> // read(), open()

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
    printf("%-8.8s ", current_record->ut_user);
    printf("%-8.8s ", current_record->ut_line);
    printf("%10d ", current_record->ut_time);
    printf("(%-s)\n", current_record->ut_host);
}
