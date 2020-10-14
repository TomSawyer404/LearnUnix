/* twordcount3.c 
 * - threaded word counter for two files
 *   Version 3: one counter per file
 **/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>

struct arg_set {    // two values in one arg
    char* fname;    // file to examine
    int count;      // number of words
};

void* count_words(void*);


int
main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: %s file1 file2\n", argv[0]);
        exit(1);
    }
    struct arg_set args1, args2;
    args1.fname = argv[1];
    args1.count = 0;
    args2.fname = argv[2];
    args2.count = 0;
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, count_words, (void*)&args1);
    pthread_create(&t2, NULL, count_words, (void*)&args2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%5d: %s\n", args1.count, argv[1]);
    printf("%5d: %s\n", args2.count, argv[2]);
    printf("%5d: total words\n", args1.count + args2.count);

    return 0;
}


void* count_words(void* a) {
    struct arg_set* argsPtr = a;
    FILE* fp;
    int c, prevc = '\0';

    if( (fp = fopen(argsPtr->fname, "r")) != NULL ) {
        while( (c = getc(fp)) != EOF ) {
            if( !isalnum(c) && isalnum(prevc) ) 
                argsPtr->count++;
            prevc = c;
        }
    } else {
        perror(argsPtr->fname);
    }
    return NULL;
}

