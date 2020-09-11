/* setecho.c
 * usage: setecho [y|n] 
 * shows: how to read, change, reset tty attributes
 **/

#include <stdio.h>
#include <termios.h>

#define oops(s,x) {perror(s); return x;}

int
main(int argc, char** argv) {
    struct termios info;
    int fd = 0;  // fileDescript connected to a terminal

    if(argc == 1)  return 0;
    if( tcgetattr(fd, &info) == -1)  oops("tcgettattr", 1);

    if( argv[1][0] == 'y')
        info.c_lflag |= ECHO;  // turn on bit
    else 
        info.c_lflag &= ~ECHO;  // turn off bit
    if( tcsetattr(fd, TCSANOW, &info) == -1 )
        oops("tcsetattr", 2);
    return 0;
}
