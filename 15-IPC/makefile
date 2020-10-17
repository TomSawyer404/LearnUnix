FLAG = -Wall -Werror -g

selectdemo.out: selectdemo.c
	gcc $^ -o $@ $(FLAG)

################################# file lock demo
fileLock: file_tc.out file_ts.out

file_tc.out: file_tc.c
	gcc $^ -o $@ $(FLAG)

file_ts.out: file_ts.c
	gcc $^ -o $@ $(FLAG)

################################### share memory demo
shareMemory: shmserv.out shmclnt.out

shmserv.out: shm_ts2.c
	gcc $^ -o $@ $(FLAG)

shmclnt.out: shm_tc2.c
	gcc $^ -o $@ $(FLAG)

#############
clean:
	rm *.out
