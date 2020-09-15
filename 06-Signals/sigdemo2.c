/* sigdemo2.c
 * - shows how to ignore a signal
 * - press Ctrl-\ to kill this one
 **/

#include <stdio.h>
#include <signal.h>  // signal()
#include <unistd.h>  // sleep()

int
main () {
    signal(SIGINT, SIG_IGN);
    printf("YOU CANNOT STOP ME!!!\n");
    while(1) {
        sleep(1);
        printf("HaHa\n");
    }

    return 0;
}
