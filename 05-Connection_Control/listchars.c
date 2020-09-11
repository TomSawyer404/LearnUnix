/* listchars.c
 * purpose: list individually all the chars seen on input
 * input: stdin, until the letter Q
 * output: char and ascii code, one pair per line
 * notes: useful to show that buffering/editing exists
 **/

#include <stdio.h>

int
main() {
    int c, n = 0;
    while( (c = getchar()) != 'Q' ) {
        printf("char %3d is %c code %d\n", n++, c, c);
    }
    return 0;
}
