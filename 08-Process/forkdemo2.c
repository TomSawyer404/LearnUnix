/* forkdemo2.c 
 * shows how the return value from fork()
 * allows a process to determine whether
 * it is a child or process
 **/

#include <stdio.h>
#include <unistd.h>

int
main() {
    printf("Before: my pid is %d\n", getpid());

    int fork_rv = fork();   // create new process
    
    if( fork_rv == -1 )  perror("fork");    // chekc for error
    else if( fork_rv == 0 )  
        printf("I am the child. my pid = %d\n", getpid());
    else
        printf("I am the parent. my child is %d\n", fork_rv);
    
    return 0;
}
