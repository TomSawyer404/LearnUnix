/* twordcount2.c
 * - threaded word counter for two files
 *   Version 2: uses mutex to lock counter
 **/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

int TotalWorlds = 0;    // the counter and its lock
pthread_mutex_t counterLock = PTHREAD_MUTEX_INITIALIZER;


int
main(int argc, char** argv) {
    pthread_t t1, t2;
    void* count_words(void*);

    if(argc != 3) {
        fprintf(stderr, "usage: %s file1 fil2\n", argv[0]);
        exit(1);
    }

    pthread_create(&t1, NULL, count_words, (void*)argv[1]);
    pthread_create(&t2, NULL, count_words, (void*)argv[2]);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%5d: total word\n", TotalWorlds);
    
    return 0;
}


void* count_words(void* f) {
    char* filename = (char*)f;
    FILE* fp;
    int c, prevC = '\0';
    if( (fp = fopen(filename, "r")) != NULL ) {
        while( (c = getc(fp)) != EOF ) {
            if( !isalnum(c) && isalnum(prevC) ) {
                pthread_mutex_lock(&counterLock);
                TotalWorlds++;
                pthread_mutex_unlock(&counterLock);
            }
            prevC = c;
        }
        fclose(fp);
    } else {
        perror(filename);
    }
    return NULL;
}

