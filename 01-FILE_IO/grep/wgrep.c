/* 
   wgrep.c - program wgrep of ostep-projects 
  - Prints the line if the specified word matched.
  - Prints NOTHING if no args inputs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void do_grep(char*, FILE*);

int main(int argc, char** argv) {

	FILE* filePointer;

	if( argc >= 3 ) {  // standerd usage `unix> ./wgrep a hello.txt`

		char searchTerm[512];
		sscanf(*++argv, "%512s", searchTerm);

		if( (filePointer = fopen(*(++argv), "r")) != NULL ) {  // Open a file successfully
			do_grep(searchTerm, filePointer);
			fclose(filePointer);
		} else {  // Fail in opening a file
			printf("wgrep: cannot open file\n");
			exit(1);
		}
	} 
	else if( argc == 2 ){  // Read from stdin redirected in another file
		char searchTerm[512];
		sscanf(*++argv, "%512s", searchTerm);
		do_grep(searchTerm, stdin);
	} else {  // No arguments
		printf("wgrep: searchterm [file ...]\n");
		exit(1);
	}

	return 0;
}


/** Prints the line if specified words matched */
void do_grep(char* searchTerm, FILE* fp) {
	char* buffer = (char*)malloc(2048); // Create a buffer, it has 2048 bytes 

	while( fgets(buffer, 2048, fp) ) {
		if( strstr(buffer, searchTerm) )
			fputs(buffer, stdout);
	}

	free(buffer);
}
