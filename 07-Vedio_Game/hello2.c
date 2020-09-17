/* hello2.c
 * purpose: show how to use curses functions with a loop
 * outline: initialize, draw stuff, wrap up
 **/

#include <stdio.h>
#include <curses.h>

int
main() {
    initscr();       // turn on curses
    clear();        // draw some stuff
    
    for(int i = 0; i < LINES; i++) {
        move(i, i + 1);
        if( i % 2 == 1 )
            standout();
        addstr("Hello World");
        if( i % 2 == 1 )
            standend();
    }
    //refresh();      // update the screen
    getch();        // wait for user input
    endwin();       // reset the tty etc

    return 0;
}
