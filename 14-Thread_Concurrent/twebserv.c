/* twebserv.c
 * - a threaded minmal web server (version 0.2) 
 *   usage: tws portNumber
 * Features: supports the GET command only
 *           runs in the current directory
 *           creates a thread to handle each request
 *           supports a special status URL to report internal state
 * building: gcc twebserv.c socklib.c -lpthread -o tws.out
 **/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

/* server facts here */
time_t serverStarted;
int serverBytesSent;
int serverRequests;

int make_server_socket(int);    // in socklib.c
void* handle_call(void*);       // thread function
void setup(pthread_attr_t*);
void skip_rest_of_header(FILE*);
void process_rg(char* rq, int fd);
void sanitize(char* str);
int built_in(char* arg, int fd);
int http_reply(int fd, FILE** fpp, int code, char* msg, char* type, char* content);
void not_implemented(int fd);
void do_404(char* item, int fd);
int isadir(char* f);
int not_exist(char* f);
void do_ls(char* dir, int fd);
char* file_type(char* f);
void do_cat(char* f, int fd);


int
main(int argc, char** argv) {
    if(argc == 1) {
        fprintf(stderr, "usage: tws portNumber\n");
        exit(1);
    }

    int sock = make_server_socket(atoi( argv[1]) );
    if(sock == -1) {
        perror("making socket");
        exit(2);
    }

    pthread_attr_t attr;
    setup(&attr);

    /** main loop here: take call, handle call in new thread */
    int* fdPtr;
    pthread_t worker;
    while(1) {
        int fd = accept(sock, NULL, NULL);
        serverRequests++;

        fdPtr = malloc(sizeof(int));
        *fdPtr = fd;
        pthread_create(&worker, &attr, handle_call, fdPtr);
    }

    return 0;
}


/* initialize the status variables and 
 * set the thread attribute to detached 
 **/
void setup(pthread_attr_t* attrPtr) {
    pthread_attr_init(attrPtr);
    pthread_attr_setdetachstate(attrPtr, PTHREAD_CREATE_DETACHED);

    time(&serverStarted);
    serverRequests = 0;
    serverBytesSent = 0;
}


void* handle_call(void* fdPtr) {
    int fd = *(int*)fdPtr;
    free(fdPtr);                        // get fd from arg

    FILE* fpin = fdopen(fd, "r");       // buffer input
    char request[BUFSIZ];
    fgets(request, BUFSIZ, fpin);       // read client request
    printf("Goa a call on %d: request = %s", fd, request);
    skip_rest_of_header(fpin);

    process_rg(request, fd);            // process client rg
    fclose(fpin);
    return NULL;
}


/* ###############################################################
 * skip_rest_of_header(FILE*) 
 * skip over all request info until a CRNL is seen
 * ############################################################### */
void skip_rest_of_header(FILE* fp) {
    char buf[BUFSIZ];
    while( fgets(buf, BUFSIZ, fp) != NULL && strcmp(buf, "\r\n") != 0 )
        ;
}


/* ###############################################################
 * process_rq(char* rq, int fd)
 * do what the request asks for and write reply to fd
 * handles request in a new process
 * rq is HTTP command: GET /foo/bar.html HTTP/1.0
 * ############################################################### */
void process_rg(char* rq, int fd) {
    char cmd[BUFSIZ], arg[BUFSIZ];
    if( sscanf(rq, "%s%s", cmd, arg) != 2 ) 
        return;
    sanitize(arg);
    printf("sanitized version is %s\n", arg);

    if( strcmp(cmd, "GET") != 0 )
        not_implemented(fd);
    else if( built_in(arg, fd) )
        ;
    else if( not_exist(arg) )
        do_404(arg, fd);
    else
        do_cat(arg, fd);
}


/* ###############################################################
 * make sure all paths are below the current directory
 * ############################################################### */
void sanitize(char* str) {
    char *src, *dest;
    src = dest = str;

    while(*src) {
        if( strncmp(src, "/../", 4) == 0 )
            src += 3;
        else if( strncmp(src, "//", 2) == 0 ) 
            src++;
        else
            *dest++ = *src++;
    }
    *dest = '\0';
    
    if( *str == '/' )
        strcpy(str, str+1);

    if( str[0] == '\0' || strcmp(str, "./") == 0
        || strcmp(str, "./..") == 0 )
        strcpy(str, ".");
}


/* handle built-in URLs here. Only one so far is "status" */
int built_in(char* arg, int fd) {
    FILE* fp;
    if( strcmp(arg, "status") != 0 )
        return 0;
    http_reply(fd, &fp, 200, "OK", "text/plain", NULL);

    fprintf(fp, "Server started: %s", ctime(&serverStarted));
    fprintf(fp, "Total requests: %d\n", serverRequests);
    fprintf(fp, "Bytes sent out: %d\n", serverBytesSent);
    fclose(fp);
    return 1;
}


int 
http_reply(int fd, FILE** fpp, int code, char* msg, char* type, char* content) {
    FILE* fp = fdopen(fd, "w");
    int bytes = 0;

    if(fp != NULL) {
        bytes = fprintf(fp, "HTTP/1.0 %d %s\r\n", code, msg);
        bytes += fprintf(fp, "Content-type: %s\r\n\r\n", type);
        if( content )
            bytes += fprintf(fp, "%s\r\n", content);
    }
    
    fflush(fp);
    if(fpp)
        *fpp = fp;
    else 
        fclose(fp);

    return bytes;
}


/* ############################################################### 
 * Simple functions first:
 *   not_implemented(fd)    unimplemented HTTP command
 *   and do_404(item, fd)   no such object 
 * ############################################################### */
void not_implemented(int fd) {
    http_reply(fd, NULL, 501, "Not Implemented", "text/plain",
            "That command is not implemented");
}


void do_404(char* item, int fd) {
    http_reply(fd, NULL, 404, "Not Found", "text/plain", 
            "The item you seek is not here");
}


/* ############################################################### 
 * the directory listing section
 * isadir() uses stat, not_exist() uses stat
 * ############################################################### */
int isadir(char* f) {
    struct stat info;
    return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode) );
}


int not_exist(char* f) {
    struct stat info;
    return ( stat(f, &info) == -1 );
}


void do_ls(char* dir, int fd) {
    FILE* fp;
    int bytes = http_reply(fd, &fp, 200, "OK", "text/plain", NULL);
    bytes += fprintf(fp, "Listing of Directory %s\n", dir);

    DIR* dirPtr = opendir(dir);
    struct dirent* direntPtr;
    if( dirPtr != NULL ) {
        while( (direntPtr = readdir(dirPtr)) != NULL ) {
            bytes += fprintf(fp, "%s\n", direntPtr->d_name);
        }
    }
    closedir(dirPtr);
    serverBytesSent += bytes;
}


/* ############################################################### 
 * functions to cat files here
 * file_type(filename) returns the 'extension': cat uses it
 * ############################################################### */
char* file_type(char* f) {
    char* cp = strrchr(f, '.');
    if( cp != NULL ) 
        return cp + 1;
    return "";
}


/* do_cat(filename, fd): sends header then the contents */
void do_cat(char* f, int fd) {
    char* extension = file_type(f);
    char* type = "text/plain";
    if( strcmp(extension, "html") == 0 ) 
        type = "text/html";
    else if( strcmp(extension, "gif") == 0 )
        type = "image/gif";
    else if( strcmp(extension, "jpg") == 0 ) 
        type = "image/jpeg";
    else if( strcmp(extension, "jpeg") == 0 )
        type = "image/jpeg";

    int c, bytes = 0;
    FILE* fpsock = fdopen(fd, "w");
    FILE* fpfile = fopen(f, "r");
    if( fpsock != NULL && fpfile != NULL ) {
        bytes = http_reply(fd, &fpsock, 200, "OK", type, NULL);
        while( (c = getc(fpfile)) != EOF ) {
            putc(c, fpsock);
            bytes++;
        }
        fclose(fpfile);
        fclose(fpsock);
    }
    serverBytesSent += bytes;
}

