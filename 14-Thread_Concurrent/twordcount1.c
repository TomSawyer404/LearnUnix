/* twordcount1.c
 * - threaded word counter for two files.
 *   Version 1
 **/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

int TotalWords = 0;

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

    printf("%5d: total words\n", TotalWords);

    return 0;
}


void* count_words(void* f) {
    char* filename = (char*)f;
    int c, prevC = '\0';
    FILE* fp;
    if( (fp = fopen(filename, "r")) != NULL ) {
        while( (c = getc(fp)) != EOF ) {
            if( !isalnum(c) && isalnum(prevC) )
                TotalWords++;
            prevC = c;
        }
        fclose(fp);
    } else {
        perror(filename);
    }
    return NULL;
}
