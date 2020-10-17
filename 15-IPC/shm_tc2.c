/* shm_tc2.c
 * - time client shared mem ver2
 * - use semaphores for locking
 * - program uses shared memory with key 99
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>

#define TIME_MEM_KEY    99              // kind of like a port number
#define TIME_SEM_KEY    9900            // like a filename
#define SEG_SIZE        ((size_t)100)   // size of segment
#define oops(m,x)   {perror(m); exit(x);}

union semun {
    int val;
    struct semid_ds* buf;
    ushort* array;
};


void release_lock(int semset_id);
void wait_and_lock(int semset_id); 


int main() {
    /** Create a shared memory segment */
    int seg_id = shmget(TIME_MEM_KEY, SEG_SIZE, 0777);
    if( seg_id == -1 )
        oops("shamt", 1);

    /** Attach to it and get a pointer to where it attaches */
    char* mem_ptr = shmat(seg_id, NULL, 0);
    if( mem_ptr == (void*)-1 )
        oops("shamt", 2);

    /** Connect to semaphore set 9900 with 2 semaphores */
    int semset_id = semget(TIME_SEM_KEY, 2, 0);
    wait_and_lock(semset_id);

    printf("The time, direct from memory: .. %s", mem_ptr);

    release_lock(semset_id);
    shmdt(mem_ptr);    // detach, buf not needed here

    return 0;
}


/* build and execute a 2-element action set
 * wait for 0 on n_writers AND increment n_readers
 **/
void wait_and_lock(int semset_id) {
    struct sembuf actions[2];

    actions[0].sem_num = 1;             // sem[1] is n_writers
    actions[0].sem_flg = SEM_UNDO;      // auto cleanup
    actions[0].sem_op = 0;              // wait for 0

    actions[1].sem_num = 0;             // sem[0] is n_readers
    actions[1].sem_flg = SEM_UNDO;      // auto cleanup
    actions[1].sem_op = +1;             // incr n_readers

    if( semop(semset_id, actions, 2) == -1 )
        oops("semop: locking", 10);
}


/* build and execute a 1-element action set
 * decrement num_readers
 **/
void release_lock(int semset_id) {
    struct sembuf actions[1];

    actions[0].sem_num = 0;
    actions[0].sem_flg = SEM_UNDO;
    actions[0].sem_op = -1;

    if( semop(semset_id, actions, 1) == -1 )
        oops("semop: unlocking", 10);
}
