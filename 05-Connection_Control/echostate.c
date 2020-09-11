/* echostate.c 
 * reports current state of echo bit in tty driver for fd 0
 * shows how to read sttributes from dirver and test a bit
 **/

#include <stdio.h>
#include <termios.h>

int
main(int argc, char** argv) {
    struct termios info;
    int fd = 0;  // fileDescript connected to a terminal
    int rv = tcgetattr(fd, &info);  // read values from driver
    if(rv == -1) {
        perror("tcgetattr");
        return 1;
    }
    if( info.c_lflag & ECHO ) {
        printf("echo is on, since its bit is 1\n");
    } else {
        printf("echo is OFF, since its bits is 0\n");
    }

    return 0;
}
