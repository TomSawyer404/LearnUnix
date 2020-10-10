/* popendemo.c
 * - de,pmstrates jpw tp open a program for standard i/o
 *   important points:
 *      1. popen() returns a FILE*, just like fopen()
 *      2. the FILE* it returns can be read/written
 *         with all the standard functions
 *      3. you need to use pclose() when done
 **/

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char** argv) {
    FILE* fp = popen("who|sort", "r");  // open the command

    char buf[100];
    int i = 0;
    while( fgets(buf, 10, fp) != NULL )  // read from command
        printf("%3d %s", i++, buf);

    pclose(fp);     // IMPORTANT!
    return 0;
}
