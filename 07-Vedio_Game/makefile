LD = -lcurses
FLAG = -Wall -Werror

h1: hello1.c
	gcc -o hello1 hello1.c $(FLAG) $(LD)

h2: hello2.c
	gcc -o hello2 hello2.c $(FLAG) $(LD)

h3: hello3.c
	gcc -o hello3 hello3.c $(FLAG) $(LD)

h4: hello4.c
	gcc -o hello4 hello4.c $(FLAG) $(LD)

h5: hello5.c
	gcc -o hello5 hello5.c $(FLAG) $(LD)

s1: sleep1.c
	gcc -o sleep1 sleep1.c $(FLAG)

ticker: ticker_demo.c
	gcc -o ticker_demo ticker_demo.c $(FLAG)

sig3: sigdemo3.c
	gcc -o sigdemo3 sigdemo3.c $(FLAG)

sigact: sigactdemo.c
	gcc -o sigactdemo sigactdemo.c $(FLAG)

bounce1d: 
	gcc bounce1d.c set_ticker.c $(LD) $(FLAG)

bounce2d: 
	gcc bounce2d.c set_ticker.c $(LD) $(FLAG)
