/* socklib.c
 *
 * This file contains functions used lots when writing internet
 * client/server programs. The two main functions here are:
 *
 * int make_server_socket(portNum) returns a server socket
 *                                  or -1 if error
 * 
 * int make_server_socket_q(portNum, backlog)
 *
 * int connect_to_server(char* hostName, int portNum) 
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <strings.h>

#define HOSTLEN     256
#define BACKLOG     1

int make_server_socket_q(int, int);


int make_server_socket(int portNum) {
    return make_server_socket_q(portNum, BACKLOG);
}


int make_server_socket_q(int portNum, int backlog) {
    int sock_id = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_id == -1)  return -1;

    /* Build address and bind it to socket */
    struct sockaddr_in saddr;               // build our address here
    bzero((void*)&saddr, sizeof(saddr));    // clear out struct

    char hostName[HOSTLEN];
    gethostname(hostName, HOSTLEN);         // where am I ?
    struct hostent* hp = gethostbyname(hostName);   // get info about host

    bcopy((void*)hp->h_addr, (void*)&saddr.sin_addr, hp->h_length);
    saddr.sin_port = htons(portNum);        // fill in socket port
    saddr.sin_family = AF_INET;             // fill in addr family
    
    if(bind(sock_id, (struct sockaddr*)&saddr, sizeof(saddr)) != 0)
        return -1;

    /* arrange for incoming calls */
    if( listen(sock_id, backlog) != 0 )
        return -1;

    return sock_id;
}


int connect_to_server(char* host, int portNum) {
    /* Step 1, Get a socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        return -1;

    /* Step 2, connect to server */
    struct sockaddr_in servadd;
    bzero(&servadd, sizeof(servadd));           // zero the address
    
    struct hostent* hp = gethostbyname(host);   // lookup host's ip #
    if(hp == NULL)
        return -1;
    
    bcopy(hp->h_addr, (struct sockaddr*)&servadd.sin_addr, hp->h_length);
    servadd.sin_port = htons(portNum);  // fill in port number
    servadd.sin_family = AF_INET;       // fill in socket type

    if( connect(sock, (struct sockaddr*)&servadd, sizeof(servadd)) != 0 )
        return -1;

    return sock;
}

