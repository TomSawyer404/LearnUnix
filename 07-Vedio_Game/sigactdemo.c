/* sigactdemo.c
 * purpose: shows use of sigaction()
 * feeature: blocks ^\ while handling ^C
 *           does not reset ^C handler, so two kill
 **/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define INPUTLEN 100

void IntHandler();


int
main() {
    struct sigaction newHandler;    // new settings
    sigset_t blocked;               // set of blocked sigs
    char x[INPUTLEN];

    /** load these two members first */
    newHandler.sa_handler = IntHandler;                 // handler function
    newHandler.sa_flags = SA_RESETHAND | SA_RESTART;    // options

    /** then build the list of bloked signals */
    sigemptyset(&blocked);          // clear all bits
    sigaddset(&blocked, SIGQUIT);   //add SIGQUIT to list
    newHandler.sa_mask = blocked;   // store blockmask

    if( sigaction(SIGINT, &newHandler, NULL) == -1 )
        perror("sigaction");
    else {
        while(1) {
            fgets(x, INPUTLEN, stdin);
            printf("input: %s", x);
        }
    }

    return 0;
}


void IntHandler(int s) {
    printf("Called with signal %d\n", s);
    sleep(s);
    printf("done handling signal %d\n", s);
}
