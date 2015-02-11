mythread.a: mythread.o threadutility.o
	ar rcs mythread.a mythread.o threadutility.o 

mythread.o: mythread.c mythread.h threadutility.c threadutility.h
	gcc -c -O2 mythread.c threadutility.c

threadutility.o: threadutility.c threadutility.h
	gcc -c -O2 threadutility.c -o threadutility.o
