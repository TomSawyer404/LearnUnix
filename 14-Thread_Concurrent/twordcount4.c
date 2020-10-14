/* twordcount4.c
 * - threaded word counter for two files.
 *   Version 4: condition variable allows counter
 *              functions to report results early
 **/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>

struct arg_set {
    char* fname;    // file to examine
    int count;      // number of words
};

struct arg_set* mailbox;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t flag = PTHREAD_COND_INITIALIZER;

void* count_words(void*);


int
main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: %s file1 fil2\n", argv[0]);
        exit(1);
    }
    
    int reports_in = 0;
    int TotalWords = 0;
    struct arg_set args1, args2;
    pthread_mutex_lock(&lock);   // lock the mailbox now
    
    args1.fname = argv[1];
    args1.count = 0;
    pthread_t t1;
    pthread_create(&t1, NULL, count_words, (void*)&args1);

    args2.fname = argv[2];
    args2.count = 0;
    pthread_t t2;
    pthread_create(&t2, NULL, count_words, (void*)&args2);

    while( reports_in < 2 ) {
        printf("MAIN: waiting for flag to go up\n");
        pthread_cond_wait(&flag, &lock);    // wait for notify

        printf("MAIN: Wow! flag was raised, I have the lock\n");
        printf("%7d: %s\n", mailbox->count, mailbox->fname);
        TotalWords += mailbox->count;

        if( mailbox == &args1 ) 
            pthread_join(t1, NULL);
        if( mailbox == &args2 )
            pthread_join(t2, NULL);
        mailbox = NULL;

        pthread_cond_signal(&flag);
        reports_in++;
    }
    printf("%7d: total words\n", TotalWords);

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
        fclose(fp);
    } else {
        perror(argsPtr->fname);
    }

    printf("COUNT: waiting to get lock\n");
    pthread_mutex_lock(&lock);      // get the mailbox
    printf("COUNT: have lock, storing data\n");
    if( mailbox != NULL )
        pthread_cond_wait(&flag, &lock);
    mailbox = argsPtr;              // put ptr to our args there

    printf("COUNT: raising flag\n");
    pthread_cond_signal(&flag);     // raise the flag
    
    printf("COUNT: unlocking box\n");
    pthread_mutex_unlock(&lock);    // release the mailbox
    
    return NULL;
}
