/* whotofile.c 
 * purpose: show how to redirect output for another program
 * idea: fork, then in the child, redirect output, then exec
 **/

#include <stdio.h>
#include <unistd.h>     // fork(), close(), execlp()
#include <fcntl.h>      // creat()
#include <sys/wait.h>

int
main() {
    printf("About to run who into a file\n");

    /** create a new process or quit */
    int pid;
    if( (pid = fork()) == -1 ) {
        perror("fork");
        return 1;
    }

    /** child does the work */
    if( pid == 0 ) {
        close(1);                           // close stdout
        int fd = creat("userlist", 0644);   // then open
        execlp("who", "who", NULL);         // and run
        perror("execlp");
        return 1;
    }

    /** parent waits then reports */
    if( pid != 0 ) {
        wait(NULL);
        printf("Done running who. results in userlist.\n");
    }
    
    return 0;
}
