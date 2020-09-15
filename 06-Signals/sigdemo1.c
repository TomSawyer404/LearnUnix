/* sigdemo1.c
 * - shows how a siganl handler works.
 * - run this and press Ctrl-C a few times.
 **/

#include <stdio.h>
#include <signal.h>  // signal()
#include <unistd.h>  // sleep()

void handler(int);


int
main() {
    signal(SIGINT, handler);
    for(int i = 0; i < 5; i++) {
        printf("Hello\n");
        sleep(1);
    }

    return 0;
}


void handler(int signum) {
    printf("OUCH!\n");
}
