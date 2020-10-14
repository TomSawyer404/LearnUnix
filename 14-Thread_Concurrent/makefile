FLAG = -Wall -Werror -g
FLAG += -lpthread

hello-single: hello_single.c
	gcc -o hello-single.out hello_single.c $(FLAG)

hello-multi: hello_multi.c
	gcc -o hello-multi.out hello_multi.c $(FLAG)

incrprint: incrprint.c
	gcc -o incrprint.out incrprint.c $(FLAG)

twordcount1: twordcount1.c
	gcc -o twc1.out twordcount1.c $(FLAG)

twordcount2: twordcount2.c
	gcc -o twc2.out twordcount2.c $(FLAG)

twordcount3: twordcount3.c
	gcc -o twc3.out twordcount3.c $(FLAG)

twordcount4: twordcount4.c
	gcc -o twc4.out twordcount4.c $(FLAG)

webserv: twebserv.c
	gcc -o tws.out twebserv.c socklib.c $(FLAG)

clean:
	rm *.out
