/* timeserv.c
 * A socket - based time of day server
 **/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define PORTNUM     13000   // our time service phone number
#define HOSTLEN     256 
#define oops(msg)   {perror(msg); exit(1);}

int
main(int argc, char** argv) {
    
    /** Step 1: ask kernel for a socket */
    int sock_id = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_id == -1) 
        oops("socket");

    /** Step 2: bind address to socket. Address is host, port */
    struct sockaddr_in saddr;                // build our address here
    bzero( (void*)&saddr, sizeof(saddr) );   // clear out struct
    
    char hostname[HOSTLEN];
    gethostname(hostname, HOSTLEN);                 // where am I?
    struct hostent* hp = gethostbyname(hostname);   // get info about host
                                                    // fill in host part
    bcopy( (void*)hp->h_addr, (void*)&saddr.sin_addr,
        hp->h_length );
    saddr.sin_port = htons(PORTNUM);        // fill in socket port
    saddr.sin_family = AF_INET;             // fill in addr family

    if( bind(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) != 0 )
        oops("bind");

    /** Step 3: allow incoming calls with Qsize = 1 on socket */
    if( listen(sock_id, 1) != 0 )
        oops("listen");

    /** main loop: accept(), write(), close() */
    while(1) {
        int sock_fd = accept(sock_id, NULL, NULL);  // wait for call
        printf("Wow! got a call!\n");
        if( sock_fd == -1 ) 
            oops("accept");     // error getting calls

        FILE* sock_fp = fdopen(sock_fd, "w");       // we'll write to the
        if( sock_fp != NULL )                       // socket as a stream
            oops("fdopen");                         // unless we can't

        time_t theTime = time(NULL);    // get time and convert to string

        fprintf(sock_fp, "The time here is...");
        fprintf(sock_fp, "%s", ctime(&theTime));
        fclose(sock_fp);
    }

    return 0;
}
