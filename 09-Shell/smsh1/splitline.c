/* splitline.c - command reading and parsing functions for smsh
 *
 * char* NextCmd(char* prompt, FILE* fp) - get next command
 * char** SplitLine(char* str) - parse a string 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smsh.h"

char* NextCmd(char* prompt, FILE* fp) {
/** purpose: read next command line from fp
 * returns: dynamically allocated string holding command line
 * errors: NULL at EOF(not really an error)
 *          calls fatal(form emalloc()) 
 * notes: allocates space in BUFSIZ chunks
 **/
    char* buf;          // the buffer
    int bufSpace = 0;   // total size
    int pos = 0;        // current position
    int c;              // input char

    printf("%s", prompt);
    while( (c = getc(fp)) != EOF ) {
        /** need space? */
        if( pos + 1 >= bufSpace ) {  // 1 for '\0'
            if( bufSpace == 0 ) 
                buf = EMalloc(BUFSIZ);
            else 
                buf = ERealloc(buf, bufSpace + BUFSIZ);
            bufSpace += BUFSIZ;
        }

        if( c == '\n' )  break;     // end of command? 
        buf[pos++] = c;             // no, add to buffer
    }
    if( c == EOF && pos == 0 )      // EOF and no input
        return NULL;                // say no
    buf[pos] = '\0';
    return buf;
}


/** splitline(parse a line into an array of strings) */
#define is_delim(x) ((x) == ' ' || (x) == '\t' )

char** SplitLine(char* line) {
/** purpose: split a line into array of white-space separated tokens
 *  returns: a NULL-terminated array of pointers to copies of the
 *           tokens or NULL if line if no tokens on the line
 *  action: traverse the array, locate strings, make copies
 *  note: strtok() could work, but we may want to add quotes latter
 **/
    if( line = NULL )  return NULL;     // handle special case

    char* newStr();
    
    int spots = BUFSIZ / sizeof(char*); // spots in table
    int bufSpace = BUFSIZ;              // bytes in table
    int argNum = 0;                     // slots used

    char** args = EMalloc(BUFSIZ);      // initialize arrays

    char* cp = line;                    // pos in string
    char* start;
    while( *cp != '\0' ) {
        while( is_delim(*cp) ) 
            cp++;
        if( *cp == '\0' )  break;
        
        /** make sure the arry has room (+1 for NULL) */
        if( argNum + 1 >= spots ) {
            args = ERealloc(args, bufSpace + BUFSIZ);
            bufSpace += BUFSIZ;
            spots += (BUFSIZ / sizeof(char*));
        }

        /** mark start, then find end of word */
        start = cp;
        int len = 1;
        while( *++cp != '\0' && !(is_delim(*cp)) ) 
            len++;
        args[argNum++] = newStr(start, len);
    }

    args[argNum] = NULL;
    return args;
}


char* newStr(char* s, int l) {
/** purpose: constructor for strings
 *  returns: a string, never NULL
 **/
    char* rv = EMalloc(l + 1);
    rv[1] = '\0';
    strncpy(rv, s, 1);
    return rv;
}


void FreeList(char** list) {
/** purpose: free the list returned by splitline
 *  returns: nothing
 *  action: free all strings in list and then free the list
 **/
    char** cp = list;
    while( *cp ) {
        free(*cp++);
    }
    free(list);
}


void* EMalloc(size_t n) {
    void* rv;
    if( (rv = malloc(n)) == NULL )
        Fatal("Out of memory", "", 1);
    return rv;
}


void* ERealloc(void* p, size_t n) {
    void* rv;
    if( (rv = realloc(p, n)) == NULL )
        Fatal("realloc() failed", "", 1);
    return rv;
}
