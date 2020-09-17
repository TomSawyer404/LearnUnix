/* bounce1d.c
 * purpose: animation with user controlled speed and direction
 * note: the handler does the animation
 *       the mainprogram reads keyboard input
 * conpile: gcc bounce1d.c set_ticker.c -lcurses -o bounce1d
 **/

#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <string.h>

#define MESSAGE "hello"
#define BLANK "     "

int row;    // current row
int col;    // current column
int dir;    // where we are going

int
main() {
    int delay;      // bigger => slower;
    int ndelay;     // new delay

    int c;                  // user input
    void Move_Msg(int);     // handler for timer
    int set_ticker(int);

    initscr();
    crmode();
    noecho();
    clear();

    row = 10;   // start here
    col = 0;
    dir = 1;
    delay = 200;

    move(row, col);     // get into position
    addstr(MESSAGE);    // draw message
    signal(SIGALRM, Move_Msg);
    set_ticker( delay );
    
    while(1) {
        ndelay = 0;
        c = getch();
        if( c == 'Q' )  break;
        if( c == ' ' )  dir = - dir;
        if( c == 'f' && delay > 2 )  ndelay = delay / 2;
        if( c == 's' )  ndelay = delay * 2;
        if( ndelay > 0 )
           set_ticker( delay = ndelay ); 
    }
    endwin();
    return 0;
}


void Move_Msg(int signum) {
    signal(SIGALRM, Move_Msg);      // reset, just in case
    move(row, col);
    addstr( BLANK );
    col += dir;             // move to new column
    move(row, col);
    addstr( MESSAGE );
    refresh();

    /** now handle borders */
    if( dir == -1 && col <= 0 ) 
        dir = 1;
    else if( dir == 1 && col + strlen(MESSAGE) >= COLS )
        dir = -1;
}
