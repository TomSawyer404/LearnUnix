/* listargs.c
 *   print the nummber of command line atgs, list the args,
 * then print a message to stderr 
 **/

#include <stdio.h>

int
main(int argc, char** argv) {
    printf("Number of args: %d, Args are: \n", argc);

    for(int i = 0; i < argc; i++) {
        printf("args[%d] %s\n", i, argv[i]);
    }
    fprintf(stderr, "This message is sent to stderr.\n");

    return 0;
}
