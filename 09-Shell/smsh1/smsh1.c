/* smsh1.c - shell version 1
 * first really useful version after prompting shell
 * this one parses the command line into strings
 * use forkm execm wait, and ignores signals
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "smsh.h"

#define DFL_PROMPT "BananaShell> "

void SetUp();

int
main() {
    char *cmdline, *prompt, **arglist;
    int result;

    prompt = DFL_PROMPT;
    SetUp();

    while( (cmdline = NextCmd(prompt, stdin)) != NULL ) {
        if( (arglist = SplitLine(cmdline)) != NULL ) {
            result = EXEcute(arglist);
            FreeList(arglist);  
        }
        free(cmdline);
    }

    return 0;
}


void SetUp() {
    /** purpose: initialize shell
     * returns: nothing. calls Fatal() if trouble 
     **/
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
}


void Fatal(char* s1, char* s2, int n) {
    fprintf(stderr, "Error: %s, %s\n", s1, s2);
    exit(n);
}

