/* waitdemo1.c - shows how parent pauses until child finishes */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define DELAY 2

void ChildMode(int);
void ParentMode(int);


int
main() {
    int newpid;
    printf("Before: my pid is %d\n", getpid());

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
    /** new process takes nap and then exits */
    printf("child %d here. will sleep for %d seconds\n", getpid(), delay);
    sleep(delay);
    printf("child done. about to exit\n");
    exit(17);
}


void ParentMode(int childpid) {
    /** parent waits for child then prints a message */
    int wait_rv = wait(NULL);
    printf("done waiting for %d. Wait returned: %d\n", childpid, wait_rv);
}
