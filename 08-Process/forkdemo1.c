/* forkdemo1.c
 * shows how fork create two process, distinguishable
 * bu the different return values from fork()
 **/

#include <stdio.h>
#include <unistd.h>

int
main() {
    int myPid = getpid();                       // who am i?
    printf("Before: my pid is %d\n", myPid);     // tell the world

    int child = fork();
    sleep(1);
    printf("After: my pid is %d, fork() said %d\n", getpid(), child);

    return 0;
}
