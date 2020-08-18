#ifndef UTMPLIB
#define UTMPLIB

int utmp_open(char*);
int utmp_reload();
struct utmp* utmp_next();
void utmp_close();

#endif
