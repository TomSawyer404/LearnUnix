/* prompting shell version 1
 * Prompts for the command and its arguments
 * Build the argument vector for the call to execvp
 * Uses execvp(), and never returns
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXARGS 20  // cmdline args
#define ARGLEN 100  // token length

char* MakeString(char*);
int Execute(char**);

int
main() {
    char* argList[MAXARGS + 1];    // an array of ptrs
    int numArgs = 0;                // index into array 
    char argBuf[ARGLEN];            // read stuff here
    
    while( numArgs < MAXARGS ) {
        printf("Arg[%d]? ", numArgs);
        if( fgets(argBuf, ARGLEN, stdin) && *argBuf != '\n' ) 
            argList[numArgs++] = MakeString(argBuf);
        else {
            if( numArgs > 0 ) {             // any args?
                argList[numArgs] = NULL;    // close list
                Execute( argList );         // do it
                numArgs = 0;                // and reset
            }
        }
    }

    return 0;
}


int Execute(char** argList) {
    /** use execvp to do it */
    execvp(argList[0], argList);    // do it
    perror("execvp failed");
    exit(1);
}


char* MakeString(char* buf) {
    /** trim off newline and create storeage for the string */
    buf[strlen(buf) - 1] = '\0';    // trim newline
    char* cp = malloc( strlen(buf) + 1 );   // get memory or die
    if( cp == NULL ) {
        fprintf(stderr, "no memory\n");
        exit(1);
    }
    strcpy(cp, buf);    // copy chars
    return cp;          // return ptr
}
