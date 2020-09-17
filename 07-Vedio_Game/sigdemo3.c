/* sigdemo3.c
 * purpose: show answers to signal questions
 * question1: does the handler stay in effect after a signal arrives? 
 * question2: what if a signalX arrives while handling signalX?
 * question3: what if a signalX arrives while handling signalY?
 * question4: what happens to read() when a signal arrives?
 **/

#include <stdio.h>
#include <string.h>  // strncmp()
#include <unistd.h>  // read()
#include <signal.h>  // signal()

#define INPUTLEN 100

void IntHandler(int);
void QuitHandler(int);

int
main(int argc, char** argv) {
    char input[INPUTLEN];
    int nChars;

    signal(SIGINT, IntHandler);
    signal(SIGQUIT, QuitHandler);

    do {
        printf("\nType a message:\n");
        nChars = read(0, input, (INPUTLEN - 1));
        if( nChars == -1 ) 
            perror("read returned an error");
        else {
            input[nChars] = '\0';
            printf("You typed: %s", input);
        } 
    }while( strncmp(input, "quit", 4) != 0 );

    return 0;
}


void IntHandler(int s) {
    printf("Received signal %d... waiting\n", s);
    sleep(2);
    printf("Leaving IntHandler\n");
}


void QuitHandler(int s) {
    printf("Received signal %d... waiting\n", s);
    sleep(3);
    printf("Leaving QuitHandler\n");
    
}
