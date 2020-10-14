/* hello_single.c 
 * a single threaded hello world program
 **/

#include <stdio.h>
#include <unistd.h>
#define NUM 5

int
main(int argc, char** argv) {
    void print_msg();
    
    print_msg("Hello");
    print_msg("World\n");

    return 0;
}


void print_msg(char* msg) {
    for(int i = 0; i < NUM; i++) {
        printf("%s", msg);
        fflush(stdout);
        sleep(1);
    }
}
