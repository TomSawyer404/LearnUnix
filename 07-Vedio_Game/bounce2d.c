/* bounce2d.c
 * bounce a character(default is 'o') around the screen
 * defined by some parameters
 *
 * user input: 's' slow down x component, 'S' slow y component
 *             'f' speed up x component, 'F' speed y component
 *             'Q' quit
 * blocks on read, but timer tick send SIGALRM caught by ball_move
 * conpile: gcc bounce2d.c set_ticker.c -lcurses -o bounce2d
 **/

#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include "bounce.h"

struct ppball the_ball;

void SetUp();
void WrapUp();
int BounceOrLose(struct ppball*);
int set_ticker(int);

int
main() {
    int c;                  // user input
    
    SetUp();

    while( (c = getchar()) != 'Q' ) {
        if( c == 'f' )  the_ball.x_ttm--;
        else if( c == 's' )  the_ball.x_ttm++;
        else if( c == 'F' )  the_ball.y_ttm--;
        else if( c == 'S' )  the_ball.y_ttm++;
    }
    WrapUp();
    return 0;
}


void SetUp() {
    /** init structure and other stuff */
    void BallMove(int);

    the_ball.y_pos = Y_INIT;
    the_ball.x_pos = X_INIT;
    the_ball.y_ttg = the_ball.y_ttm = Y_TTM;
    the_ball.x_ttg = the_ball.x_ttm = X_TTM;
    the_ball.y_dir = 1;
    the_ball.x_dir = 1;
    the_ball.symbol = DFL_SYMBOL;

    initscr();
    noecho();
    crmode();

    signal(SIGINT, SIG_IGN);
    mvaddch(the_ball.y_pos, the_ball.x_pos, the_ball.symbol);
    refresh();
    
    signal(SIGALRM, BallMove);
    set_ticker( 1000 / TICKS_PER_SEC );  // send millisecs per tick
}


void WrapUp() {
    set_ticker(0);
    endwin();       // put back to normal
}


void BallMove(int signum) {
    int y_cur, x_cur, moved;
    signal(SIGALRM, SIG_IGN);       // don't get caught now
    y_cur = the_ball.y_pos;
    x_cur = the_ball.x_pos;
    moved = 0;

    if( the_ball.y_ttm > 0 && the_ball.y_ttg-- == 1 ) {
        the_ball.y_pos += the_ball.y_dir;   // move
        the_ball.y_ttg = the_ball.y_ttm;    // reset
        moved = 1;
    }

    if( moved ) {
        mvaddch(y_cur, x_cur, BLANK);
        mvaddch(y_cur, x_cur, BLANK);
        mvaddch(the_ball.y_pos, the_ball.x_pos, the_ball.symbol);
        BounceOrLose(&the_ball);
        move(LINES-1, COLS-1);
        refresh();
    }
    signal(SIGALRM, BallMove);
}


int BounceOrLose(struct ppball* bp) {
    int returnVal = 0;

    if( bp->y_pos == TOP_ROW ) {
        bp->y_dir = 1;
        returnVal = 1;
    } else if( bp->y_pos == BOT_ROW ) {
        bp->y_dir = -1;
        returnVal = 1;
    }
    if( bp->x_pos == LEFT_EDGE ) {
        bp->x_dir = 1;
        returnVal = 1;
    } else if( bp->x_pos == RIGHT_EDGE ) {
        bp->x_pos = -1;
        returnVal = 1;
    }

    return returnVal;
}

