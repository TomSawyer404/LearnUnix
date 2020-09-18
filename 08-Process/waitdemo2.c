/* waitdemo2.c - shows how parent gets child status */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define DELAY 5

void ChildMode(int);
void ParentMode(int);

int
main() {
    printf("before: my pid is %d\n", getpid());
    int newpid;

    if( (newpid = fork()) == -1 ) {
        perror("fork");
    } else if( newpid == 0 ) {
        ChildMode(DELAY); 
    } else {
        ParentMode(newpid);
    }

    return 0;
}


void ChildMode(int delay) {
    /** new process takes a nap and then exits */
    printf("child %d here. will sleep for %d seconds\n", getpid(), delay);
    sleep(delay);
    printf("child done. about to exit\n");
    exit(17);
}


void ParentMode(int childPid) {
    /** parent waits for child then prints a message */
    int childStatus;
    int waitRv = wait(&childStatus);
    printf("done waiting for %d. Wait returned: %d\n", childPid, waitRv);

    int high8 = childStatus >> 8;       // 1111 1111 0000 0000
    int low7 = childStatus & 0x7f;      // 0000 0000 0111 1111
    int bit7 = childStatus & 0x80;      // 0000 0000 1000 0000
    printf("status: exit = %d, sig = %d, core = %d\n", high8, low7, bit7);
}
