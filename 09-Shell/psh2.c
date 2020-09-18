/* prompting shell version 2
 *
 * Solves the 'one-shot' problem of version 1
 *   Uses execvp(), but fork()s first so that the 
 *   shell waits around to perform another command
 * New problem: shell catches signals. Run vi, press ^c
 **/

#include <stdio.h>
#include <stdlib.h>     // exit()
#include <unistd.h>     // fork()
#include <string.h>     // strlen()
#include <sys/wait.h>   // wait()

#define MAXARGS 20      // cmdline args
#define ARGLEN 100      // token length

char* MakeString();
void Execute(char**);

int
main() {
    int numArgs = 0;                // index into array
    char* argList[MAXARGS + 1];     // an array of ptrs
    char argBuf[ARGLEN];            // read stuff here
    
    while( numArgs < MAXARGS ) {
        printf("Arg[%d]? ", numArgs);
        if( fgets(argBuf, ARGLEN, stdin) && *argBuf != '\n' ) {
            argList[numArgs++] = MakeString(argBuf);
        } else {
            if( numArgs > 0 ) {             // any args?
                argList[numArgs] = NULL;    // close list
                Execute( argList );         // do it
                numArgs = 0;                // and reset
            }
        }
    }
    
    return 0;
}


void Execute(char* argList[]) {
    /** use fork and execvp and wait to do it */
    int exitStatus;
    int pid = fork();       // make new process
    
    switch(pid) {
        case -1:
            perror("fork failed");  exit(1);
        case 0:
            execvp(argList[0], argList);
            perror("execvp failed");
            exit(1);
        default: 
            while( wait(&exitStatus) != pid );
            printf("child exited with status %d, %d\n", 
                    exitStatus >> 0, exitStatus & 0377);
    }
}


char* MakeString(char* buf) {
    /** trim off newline and create storage for the string */
    buf[strlen(buf) - 1] = '\0';            // trim newline
    char* cp = malloc( strlen(buf) + 1 );
    if( cp == NULL ) {
        fprintf(stderr, "no memory\n");
        exit(1);
    }
    strcpy(cp, buf);
    return cp;
}
