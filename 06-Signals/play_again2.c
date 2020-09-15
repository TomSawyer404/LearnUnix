/* play_again2.c
 * purpose: ask if user wants another transaction
 * method:  set tty into char-by-char mode, no-echo mode
 *          set tty into no-delay mode
 *          read char, return result
 * returns: 0 => yes, 1 => no, 2 => timeout
 * better: reset terminal mode on interrupt
 **/

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>     // strchr()
#include <ctype.h>      // tolower()
#include <unistd.h>     // sleep()

#define ASK "Do you want another transaction"
#define TRIES 3             // max tries
#define SLEEPTIME 2         // time per try
#define BEEP putchar('\a')  // alert user

int GetResponse(char*, int);
void SetCrNoEchoMode(void);
void SetNoDelayMode(void);
char GetOKchar(void);
int tty_mode(int);

int 
main(int argc, char** argv) {
    tty_mode(0);                                // save tty mode
    SetCrNoEchoMode();                          // set char-by-char mode
    SetNoDelayMode();                           // noinput => EOF
    int response = GetResponse(ASK, TRIES);     // get some answer
    tty_mode(1);                                // restore tty mode

    return response;
}


int GetResponse(char* question, int maxtries) {
    /** 
     * purpose: ask a question and wait for a y/n answer or maxtries
     * method: use getchar and complain about non y/n answers
     * returns: 0 => yes, 1 => no
     **/
    int input;
    printf("%s (y/n)?", question);      // ask
    fflush(stdout);                     // force output
    while(1) {
        sleep(SLEEPTIME);
        input = tolower(GetOKchar());   // ger next char
        if( input == 'y' )
            return 0;
        if( input == 'n' )
            return 1;
        if( maxtries-- == 0 )           // outatime?
            return 2;                   // sayso
        BEEP;
    }
}


char GetOKchar() {
    /** skip over non-legal chars and return y, Y, n, N or EOF */
    int c;
    while( (c = getchar()) != EOF && strchr("yYnN", c) == NULL ) 
        ;
   
    return c;
}


void SetCrNoEchoMode() {
    /** 
     * purpose: put file descriptor 0(i.e. stdin) into char-by-char mode
     * method: use bits in termios
     **/
    struct termios ttystate;
    tcgetattr(0, &ttystate);            // read current setting
    ttystate.c_lflag &= ~ICANON;        // no buffering
    ttystate.c_lflag &= ~ECHO;          // no echo either
    ttystate.c_cc[VMIN] = 1;            // get 1 char at a time
    tcsetattr(0, TCSANOW, &ttystate);   // install setting now
}


void SetNoDelayMode() {
    /**
     * purpose: put file descroptor 0 into no-delay mode
     * method: use fcntl to set bits
     * notes: tcsetattr() wo;; dp something similar, but it is complicated
     **/

    int termFlags;
    termFlags = fcntl(0, F_GETFL);  // read curr.settings
    termFlags |= O_NDELAY;          // flip on nodelay bit
    fcntl(0, F_SETFL, termFlags);   // and install 'em
}


int tty_mode(int how) {
    /** how == 0 ==> save current mode, how == 1 ==> restore mode 
     * this version handles termios and fcntl flags
     **/

    static struct termios originalMode;
    static int orignalFlags;
    if(how == 0) { 
        tcgetattr(0, &originalMode);
        orignalFlags = fcntl(0, F_GETFL);
    }
    else {
        tcsetattr(0, TCSANOW, &originalMode);
        fcntl(0, F_SETFL, orignalFlags);
    }
    return 0;
}

