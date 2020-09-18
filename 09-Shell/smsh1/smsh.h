#define YES  1
#define NO   0

char* NextCmd(char*, FILE*);
char** SplitLine(char*);

void FreeList(char**);
void* EMalloc(size_t);
void* ERealloc(void*, size_t);

int EXEcute(char**);
void Fatal(char*, char*, int);
