/* showtty.c
 * displays some current tty settings
 **/

#include <stdio.h>
#include <termios.h>

void showbaud(int);
void show_some_flags(struct termios*);

int
main() {
    struct termios ttyinfo;     // This struct holds tty info
    if( tcgetattr(0, &ttyinfo) == -1 ) {
        perror("cannot get params abou stdin");
        return 1;
    }

    /** show info */
    showbaud( cfgetospeed(&ttyinfo) );  // get + show baud rate
    printf("The erase character is ascii %d, Ctrl-%c\n",
            ttyinfo.c_cc[VERASE], ttyinfo.c_cc[VERASE] - 1 + 'A');
    printf("The line kill character is ascii %d, Ctrl-%c\n",
            ttyinfo.c_cc[VKILL], ttyinfo.c_cc[VKILL] - 1 + 'A');
    show_some_flags(&ttyinfo);  // show misc.flags

    return 0;
}


/** prints the speed in english */
void showbaud(int speed) {
    printf("the baud rate is ");
    switch(speed) {
        case B300:  
            printf("300\n"); 
            break;
        case B600:
            printf("600\n"); 
            break;
        case B1200:
            printf("1200\n"); 
            break;
        case B1800:
            printf("1800\n"); 
            break;   
        case B2400:
            printf("2400\n"); 
            break;
        case B4800:
            printf("4800\n"); 
            break;
        case B9600:
            printf("9600\n"); 
            break;
        default:
            printf("Fast\n");  break;
    }
}


struct flaginfo {
    int fl_value;
    char* fl_name;
};

struct flaginfo input_flags[] = {
    {IGNBRK,  "Ignore break condition"},
    {BRKINT,  "Signal interrupt on break"},
    {IGNPAR,  "Ignore chars with parity errors"},
    {PARMRK,  "Mark parity errors"},
    {INPCK,  "Enable input parity check"},
    {ISTRIP,   "Strip character"},
    {INLCR,  "Map NL to CR on input"},
    {IGNCR,  "Ignore CR"},
    {ICRNL,  "Map CR to NL on input"},
    {IXON,  "Enable start/stop output control"},
    // {_IXANY,  "enable any char to restart output"},
    {IXOFF,  "Enable start/stop input control"},
    {0,  NULL}
};

struct flaginfo local_flags[] = {
    {ISIG,  "Enable signals"},
    {ICANON,  "Canonical input(srase and kill)"},
    // {XCASE,  "Canonical upper/lower apperance"},
    {ECHO,  "Enable echo"},
    {ECHOE,  "Echo ERASE as BS-SPACE-BS"},
    {ECHOK,  "Echo KILL by starting new line"},
    {0,  NULL}
};


void show_flagset(int value, struct flaginfo theBitNames[]) {
    /** Check each bit pattern and dispay descriptive title */
    for(int i = 0; theBitNames[i].fl_value; i++) {
        printf("%s is ", theBitNames[i].fl_name);
        if( value & theBitNames[i].fl_value )
            printf("ON\n");
        else
            printf("OFF\n");
    }
}


void show_some_flags(struct termios* ttyPointer) {
    /** show the values of two of the flag set_: c_iflag and c_lflag
     * adding c_oflag and c_cflag is pretty routine - just add new
     * tables above and a bit more code below
     **/
    show_flagset( ttyPointer->c_iflag, input_flags );
    show_flagset( ttyPointer->c_lflag, local_flags );
}

