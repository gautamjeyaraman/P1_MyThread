#include "mythread.h"
#include <ucontext.h>
#include "threadutility.h"
#include <stdlib.h>
#include <stdio.h>

int threadID = 0;
int threadCounter = 1;
int waitCount = 0;

myThread *curThread = NULL, *mainThread = NULL;
ucontext_t *mainContext;
threadQueues *readyQueue = NULL, *blockedQueue = NULL;
semaphore *semaphoreQ = NULL;

void newQueue() {
	threadID = 0;
	threadCounter = 1;
	waitCount = 0;
	readyQueue = (threadQueues *) malloc(sizeof(threadQueues));
	readyQueue->head = NULL;
	blockedQueue = (threadQueues *) malloc(sizeof(threadQueues));
	blockedQueue->head = NULL;
}

void MyThreadInit(void (*start_funct)(void *), void *args) {
	mainContext = (ucontext_t*) malloc(sizeof(ucontext_t));
	myThread *localThread = (myThread*) malloc(sizeof(myThread));
	ucontext_t *temp = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(temp);
	newQueue();
	temp->uc_stack.ss_size = STACK_SIZE;
	temp->uc_stack.ss_sp = (char *) malloc(STACK_SIZE);
	temp->uc_link = 0;
	makecontext(temp, (void (*)()) start_funct, 1, args);
	localThread->threadId = threadID;
	localThread->parent = NULL;
	localThread->waitForAll = 0;
	localThread->toExit = 0;
	localThread->threadContext = *temp;
	threadQueues *child = (threadQueues*) malloc(sizeof(threadQueues));
	child->head = NULL;
	localThread->children = child;
	threadID++;
	mainThread = curThread = localThread;
	swapcontext(&mainContext, &(curThread->threadContext));
}

MyThread MyThreadCreate(void (*start_funct)(void *), void *args) {
	myThread *createThread = (myThread*) malloc(sizeof(myThread));
	ucontext_t *temp = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(temp);
	temp->uc_stack.ss_size = STACK_SIZE;
	temp->uc_stack.ss_sp = (char *) malloc(STACK_SIZE);
	temp->uc_link = 0;
	makecontext(temp, (void (*)()) start_funct, 1, args);
	threadQueues *child = (threadQueues*) malloc(sizeof(threadQueues));
	child->head = NULL;
	createThread->children = child;
	createThread->threadId = threadID;
	createThread->waitForAll = 0;
	createThread->toExit = 0;
	createThread->threadContext = *temp;
	threadID++;
	createThread->parent = curThread;
	push(curThread->children, createThread);
	push(readyQueue, createThread);
	return (MyThread) createThread;
}

void MyThreadYield(void) {
	myThread *toYield = curThread;
	curThread = pop(readyQueue);
	if (curThread == NULL) {
		curThread = toYield;
		return;
	}
	push(readyQueue, toYield);
	swapcontext(&(toYield->threadContext), &(curThread->threadContext));
}

int MyThreadJoin(MyThread thread) {
	myThread *child = (myThread*) thread;
	myThread *toJoin = curThread;
	int isAlive = 0;
	if(!(isThreadInQueue(curThread->children,child))){
		return -1;
	}
	if (isThreadInQueue(readyQueue, child)) {
		isAlive = 1;
	} else if (isThreadInQueue(blockedQueue, child)) {
		isAlive = 1;
	} else if (isThreadInQueue(semaphoreQ->semBlocked, child)) {
		isAlive = 1;
	} else {
		isAlive = 0;
	}
	if (isAlive) {
		toJoin->waitingFor = child;
		push(blockedQueue, toJoin);
		curThread = pop(readyQueue);
		if (curThread != NULL)
			swapcontext(&(toJoin->threadContext), &(curThread->threadContext));
		else {
			return -1;
		}
	}
	return 0;
}

void MyThreadJoinAll(void) {
	myThread *joinAll = curThread;
	if (joinAll->children->head == NULL) {
		return;
	} else {
		joinAll->waitForAll = 1;
		push(blockedQueue, joinAll);
		curThread = pop(readyQueue);
		swapcontext(&(joinAll->threadContext), &(curThread->threadContext));
	}
}

void MyThreadExit(void) {
	myThread *parentThread = curThread->parent;
	curThread->toExit = 1;
	myThread *toExit = curThread;
	int exitBlocked = 0;
	int gotFromBlocked = 0;
	gotFromBlocked = checkparentandclear(parentThread);
	removeparentlink(readyQueue,curThread);
	removeparentlink(blockedQueue,curThread);
	if (gotFromBlocked) {
		push(readyQueue,curThread);
		curThread = pop(readyQueue);
		setcontext(&(curThread->threadContext));
	}else{
		curThread = pop(readyQueue);
		if (curThread == NULL) {
			curThread = pop(blockedQueue);
			while (curThread != NULL) {
				if (curThread->toExit == 1 ){
					if(curThread->children->head == NULL){
						free((curThread->threadContext).uc_stack.ss_sp);
						free(curThread);
						toExit = NULL;
					}else{
						break;
					}
				}else if(curThread->waitForAll == 1){
				}else if(curThread->waitingFor != NULL){
				}else if(curThread->children->head == NULL){
					break;
				}
				curThread = pop(blockedQueue);
			}
			if (curThread == NULL) {
				setcontext(&mainContext);
			} else {
				setcontext(&(curThread->threadContext));
			}
		} else {
			if (exitBlocked){
				swapcontext(&(toExit->threadContext),&(curThread->threadContext));
			} else {
				setcontext(&(curThread->threadContext));
			}
		}
	}
}

int checkparentandclear(myThread *parentThread) {
	int gotFromBlocked = 0;
	myThread *toExit = curThread;
	myThread *temp;
	if (parentThread != NULL) {
		temp = SearchandRemove(parentThread->children, toExit);
		toExit->parent = NULL;
		if (isThreadInQueue(blockedQueue, parentThread) == 1) {
			temp = SearchandRemove(blockedQueue, parentThread);
			if (temp->waitingFor != NULL) {
				if (temp->waitingFor->threadId == toExit->threadId) {
					curThread = temp;
					curThread->waitingFor = NULL;
					gotFromBlocked = 1;
				}else{
					push(blockedQueue, temp);
					displayQ(blockedQueue);
				}
			} else if (temp->waitForAll == 1) {
				SearchandRemove(temp->children, toExit);
				if (temp->children->head == NULL) {
					temp->waitForAll = 0;
					curThread = temp;
					gotFromBlocked = 1;
				} else {
					push(blockedQueue, temp);
				}
			} else if (temp->children->head == NULL) {
				if (temp->toExit == 1) {
					free((curThread->threadContext).uc_stack.ss_sp);
					free(curThread);
					toExit = NULL;
					curThread = temp;
					gotFromBlocked = checkparentandclear(curThread->parent);
				} else {
					exit(1);
				}
			}
			else {
				push(blockedQueue, temp);
			}
		}
	}
	if (toExit != NULL) {
		free((toExit->threadContext).uc_stack.ss_sp);
		free(toExit);
		toExit = NULL;
	}
	return gotFromBlocked;
}

MySemaphore MySemaphoreInit(int initialValue) {
	if(initialValue < 0){
		return NULL;
	}
	semaphoreQ = (semaphore*) malloc(sizeof(semaphore));
	semaphoreQ->semCount = initialValue;
	semaphoreQ->initValue = initialValue;
	threadQueues *blockTh = (threadQueues*) malloc(sizeof(threadQueues));
	blockTh->head = NULL;
	semaphoreQ->semBlocked = blockTh;
	threadQueues *waitTh = (threadQueues*) malloc(sizeof(threadQueues));
	waitTh->head = NULL;
	semaphoreQ->waitingthreads = waitTh;
	return (MySemaphore) semaphoreQ;
}

void MySemaphoreSignal(MySemaphore sem) {
	semaphoreQ = (semaphore*) sem;
	if(semaphoreQ == NULL){
		return;
	}
	if (semaphoreQ->semCount <= 0) {
		myThread *temp = pop(semaphoreQ->semBlocked);
		if (temp != NULL) {
			push(readyQueue, temp);
		}
	}
	(semaphoreQ->semCount)++;
}

void MySemaphoreWait(MySemaphore sem) {
	semaphoreQ = (semaphore*) sem;
	if(semaphoreQ == NULL){
		return;
	}
	if (semaphoreQ->semCount > 0) {
	} else {
		push(semaphoreQ->semBlocked, curThread);
		myThread *temp = curThread;
		curThread = pop(readyQueue);
		if (curThread == NULL) {
			setcontext(&mainContext);
			return;
		}
		swapcontext(&(temp->threadContext), &(curThread->threadContext));
	}
	(semaphoreQ->semCount)--;
}

int MySemaphoreDestroy(MySemaphore sem) {
	semaphoreQ = (semaphore*) sem;
	if(semaphoreQ != NULL){
		if (semaphoreQ->semCount > 0){
			free(semaphoreQ->semBlocked);
			free(semaphoreQ->waitingthreads);
			semaphoreQ = NULL;
			return 0;
		}else
			return -1;
	}else{
	}
	return -1;
}
