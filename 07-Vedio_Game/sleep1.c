/* sleep1.c
 * purpose: show how sleep works
 * usage: sleep1
 * outline: sets handler, sets alarm, pauses, then returns
 **/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>  // alarm(), sleep()

// #define SHHHH

void WakeUp(int);

int
main() {
    printf("about to sleep for 4 seconds\n");
    signal(SIGALRM, WakeUp);        // catch it
    alarm(4);                       // set clock
    pause();                        // freeze here
    printf("Morning so soon?\n");   // back to work
    return 0;
}


void WakeUp(int signum) {
#ifndef SHHHH
    printf("Alarm received from kernel\n");
#endif
}
