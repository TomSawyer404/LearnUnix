/* more02.c - version 2 of more
 * - read and print 24 lines then pause for a few special commands
 * - reads from /dev/tty for commands
 **/

#include <stdio.h>
#include <stdlib.h>

#define PAGELEN 24

void do_more(FILE*);
int see_more(FILE*);

int
main(int argc, char** argv) {
    FILE* filePointer;
    if( argc == 1 )
        do_more(stdin);
    else {
        while( --argc ) {
            if( (filePointer = fopen(*++argv, "r")) != NULL ) {  // Open a file successfully
                do_more(filePointer);
                fclose(filePointer);
            } else {
                perror("more02");  // Print error info
                exit(1);
            }
        }
    }

    return 0;
}


void do_more(FILE* fp) {
/** 
 * Read PAGELEN line, then call see_more() 
 * for furher instructions
 * */

    char buffer[1024];
    int numOfLines = 0;
    
    FILE* fp_tty;  // Read commands from /dev/tty
    if( (fp_tty = fopen("/dev/tty", "r")) == NULL ) {
        perror("fp_tty");
        exit(1);
    }  

    while( fgets(buffer, 1024, fp) ) {
        if( numOfLines == PAGELEN ) {       // Full screen?
            int reply = see_more(fp_tty);   // Get reply from user
            if( reply == 0 ) {              // Reply is no
                break;
            }
            numOfLines -= reply;            // Reset count
        }
        if( fputs(buffer, stdout) == EOF )
            exit(1);
        numOfLines++;
    }
}


int see_more(FILE* fp) {
/** 
 * Print message, wait for response, return # of lines to advance
 * q means no, space means yes, CR means one line
 **/

    printf("\033[7m more? \033[m");  // Display [more?]

    int c;
    while( (c = fgetc(fp)) != EOF ) {  // Get reply
        switch(c) {
            case 'q':
                return 0;        // quit
            case ' ':
                return PAGELEN;  // Next page
            case '\n':
                return 1;        // Next line
        }
    }
    return 0;
}
