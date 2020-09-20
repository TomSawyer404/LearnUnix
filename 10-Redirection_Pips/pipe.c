/* pipe.c
 * Demonstrate how to  to create a pipeline from one process to
 *  another.
 * * Takes two args, each a command, and connects
 *   argv[1] output to input of argv[2]
 * * usage: pipe command1 command2
 *   effect: command1 | command2
 * * Limitations: commands do not take arguments
 * * uses execlp() since known number of args
 * * Note: exchange child and parent and watch fun
 **/

#include <stdio.h>
#include <unistd.h>

#define oops(m,x) {perror(m); return x;}

int
main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: pipe cmd1 cmd2\n");
        return 1;
    }

    int thePipe[2];
    if( pipe(thePipe) == -1 ) 
        oops("Cannot get a pipe", 1);
    
    /**********************************/
    /** now we have a pipe, now lets get two processes */
    int pid = fork();
    if( pid == -1 )  
        oops("Cannot fork", 2);

    /**********************************/
    /** Right Here, there are two processes
     *    parent will read from pipe */
    if( pid > 0 ) {             // parent will exec argv[2]
        close(thePipe[1]);      // parent doesn't write to pipe

        if( dup2(thePipe[0], 0) == -1 ) 
            oops("could not redirect stdin", 3);

        close(thePipe[0]);      // stdin is duped, close pipe
        execlp(argv[2], argv[2], NULL);
        oops(argv[2], 4);
    }
    
    /** child execs argv[1] and writes into pipe */
    close(thePipe[0]);          // child doesn't read from pipe

    if( dup2(thePipe[1], 1) == -1 )
        oops("could not redirect stdout", 4);
    
    close(thePipe[1]);
    execlp(argv[1], argv[1], NULL);
    oops(argv[1], 5);

    return 0;
}
