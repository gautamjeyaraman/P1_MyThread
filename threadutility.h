#ifndef THREADUTILITY_H
#define THREADUTILITY_H

#define  STACK_SIZE  8 * 1024

typedef struct myThread{
    ucontext_t threadContext;
    int threadId;
    int waitForAll;
    int toExit;
    struct myThread *parent;
    struct threadQueues *children;
    struct myThread *waitingFor;
}myThread;

typedef struct queue{
	struct myThread *readyThread;
	struct queue *next;
}queue;

typedef struct threadQueues{
	struct queue *head;
}threadQueues;

typedef struct semaphore{
	int semCount;
	int initValue;
	struct threadQueues *semBlocked;
	struct threadQueues *waitingthreads;
}semaphore;

void newQueue();

myThread* pop(struct threadQueues *threadQueue);

void push(struct threadQueues *threadQueue, myThread *readyThread);

int isThreadInQueue(struct threadQueues *threadQueue, myThread *joinThread);

myThread* SearchandRemove(struct threadQueues *threadQueue,myThread *readyThread);

int removeparentlink(struct threadQueues *threadQueue,myThread *readyThread);

myThread* popWait(struct threadQueues *threadQueue);

void pushWait(struct threadQueues *threadQueue, myThread *readyThread);

void displayQ(struct threadQueues *threadQueue);

#endif
