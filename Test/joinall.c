#include <stdio.h>
#include "mythread.h"
#include "assert.h" 

int count; 
int mode; 
MyThread T; 


void t1(void * who)
{
	int i;
	int re; 

	printf("t%d start\n",(int)who);
	if((int)who==0){
		for(i=0;i<count;i++){
			T = MyThreadCreate(t1,(void *)(i+1)); 
		}

		if (mode==1){
			printf("JoinAll\n");
			MyThreadJoinAll();
			printf("JoinAll finish\n"); 
		}
		else if (mode==2){
			MyThreadYield();
			printf("JoinAll\n"); 
			MyThreadJoinAll();
			printf("JoinAll finish\n");  
		}

	} else if(((int)who==count) && (mode==3)){
		//T contains the reference to the last child created
		//who == count indicates last child
		//So the join is an attempted on self
		re= MyThreadJoin(T);
		//printf("re %d\n",re); 
		assert(re == -1);
		//program aborts if -1 is not returned
	}

	printf("t%d end\n",(int)who);
	MyThreadExit();
}

void large_threads(void * who)
{
	int i;
	if((int)who==0){
		for(i=0;i<count;i++){
			T = MyThreadCreate(t1,(void *)(i+1)); 
		}
	}
	MyThreadExit();
	printf("end\n");
} 

void terminateNormally()
{
	printf("\nNormal termination");
}

int main(int argc, char *argv[])
{
	if(argc<2 || argc > 3)
		return -1; 
	count=atoi(argv[1]); 
	if (argc==3)
		mode=atoi(argv[2]);

	if(count < 10000) {
		MyThreadInit(t1,(void*)0);
	} else {
		printf("start\n");
		atexit(terminateNormally);
		MyThreadInit(large_threads,(void*)0);
		printf("end\n");
	}
} 


