/* hello_multi.c 
 * - a multi-threaded hello world program
 **/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM 5

int
main() {
    pthread_t t1, t2;   // 2 threads

    void* print_msg(void*);

    pthread_create(&t1, NULL, print_msg, (void*)"Hello");
    pthread_create(&t2, NULL, print_msg, (void*)"World\n");
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}

void* print_msg(void* msg) {
    char* cp = (char*)msg;
    for(int i = 0; i < NUM; i++) {
        printf("%s", cp);
        fflush(stdout);
        sleep(1);
    }
    return NULL;
}
