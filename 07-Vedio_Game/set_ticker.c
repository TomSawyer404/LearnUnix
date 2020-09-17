#ifndef SETTICKER
#define SETTICKER

#include <sys/time.h>
#include <stddef.h>

int set_ticker(int n_micorSecs) {
    struct itimerval new_timeset;
    long n_sec = n_micorSecs / 1000;
    long n_usecs = (n_micorSecs % 1000) * 1000L;

    new_timeset.it_interval.tv_sec = n_sec;     // set reload
    new_timeset.it_interval.tv_usec = n_usecs;  // new ticker value

    new_timeset.it_value.tv_sec = n_sec;        // store this
    new_timeset.it_value.tv_usec = n_usecs;     // and this

    return setitimer(ITIMER_REAL, &new_timeset, NULL);

}   

#endif
