/* exec1.c - shows how easy it is for a program to run a program
 **/

int
main() {
    char* argList[3];
    argList[0] = "ls";
    argList[1] = "-l";
    argList[2] = 0;

    printf("*** About to exec ls -l\n");
    execvp("ls", argList);
    printf("*** ls is done. bye\n");
 
    return 0;
}
