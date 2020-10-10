/* timeclnt.c - a client for timeserv.c
 * Usage: timeclnt hostname portnumber
 **/

#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>

#define oops(msg)  {perror(msg); exit(1);}

int
main(int argc, char** argv) {
    
    /* Step 1: Get a socket */
    int sock_id = socket(AF_INET, SOCK_STREAM, 0);      // get a line
    if(sock_id == -1)
        oops("socket");

    /* Step 2: connect to server
     *       need to build address (host, port) of server first
     **/
    struct sockaddr_in servadd;     // the number to call
    bzero(&servadd, sizeof(servadd));  // zero the address

    struct hostent* hp = gethostbyname(argv[1]);  // lookup hosts ip #
    if(hp == NULL)
        oops(argv[1]);

    bcopy(hp->h_addr, (struct sockaddr*)&servadd.sin_addr, hp->h_length);

    servadd.sin_port = htons(atoi(argv[2]));     // fill in port number
    servadd.sin_family = AF_INET;                // fill in socket type

    if( connect(sock_id, (struct sockaddr*)&servadd, sizeof(servadd)) != 0 )
        oops("connect");

    /* Step 3: transfet data from server, then hangup */
    char message[BUFSIZ];
    int messlen = read(sock_id, message, BUFSIZ);
    if(messlen == -1)
        oops("read");
    if( write(1, message, messlen) != messlen )
        oops("write");
    close(sock_id);

    return 0;
}
