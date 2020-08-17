/* more01.c - version 1 of more
 * - read and print 24 lines then pause for a few special commands
 **/

#include <stdio.h>
#include <stdlib.h>

#define PAGELEN 24

void do_more(FILE*);
int see_more();

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
                perror("more01");  // Print error info
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

    while( fgets(buffer, 1024, fp) ) {
        if( numOfLines == PAGELEN ) {       // Full screen?
            int reply = see_more();         // Get reply from user
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


int see_more() {
/** 
 * Print message, wait for response, return # of lines to advance
 * q means no, space means yes, CR means one line
 **/

    printf("\033[7m more? \033[m");  // Display [more?]

    int c;
    while( (c = getchar()) != EOF ) {  // Get reply
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
