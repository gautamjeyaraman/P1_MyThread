/* Reference : http://stackoverflow.com/questions/21207375/nested-linked-list-in-c */

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "threadutility.h"

myThread* pop(struct threadQueues *threadQueue) {
	myThread *node;
	if (threadQueue->head == NULL) {
		node = NULL;
	} else {
		node = threadQueue->head->readyThread;
		if (threadQueue->head->next != NULL) {
			threadQueue->head = threadQueue->head->next;
		} else {
			free(threadQueue->head);
			threadQueue->head = NULL;
		}
	}
	return node;
}

void push(struct threadQueues *threadQueue, myThread *readyThread) {
	struct queue *localQ = (struct queue*) malloc(sizeof(struct queue));
	localQ->readyThread = readyThread;
	localQ->next = NULL;
	if (threadQueue->head == NULL) {
		threadQueue->head = localQ;
	} else {
		struct queue *temp = threadQueue->head;
		while(temp->next != NULL){
			temp = temp->next;
		}
		temp->next = localQ;
	}
}

int isThreadInQueue(struct threadQueues *threadQueue, myThread *joinThread) {
	struct queue *temp = threadQueue->head;
	if (temp == NULL)
		return 0;
	else {
		while (temp != NULL) {
			if (temp->readyThread->threadId == joinThread->threadId)
				return 1;
			temp = temp->next;
		}
	}
	return 0;
}

myThread* SearchandRemove(struct threadQueues *threadQueue,	myThread *readyThread) {
	myThread *node = NULL;
	struct queue *current, *prev;
	current = prev = threadQueue->head;
	int i = 0;
	while (current != NULL) {
		if(current->readyThread->threadId == readyThread->threadId){
			node = current->readyThread;
			if (current == threadQueue->head) {
				threadQueue->head = threadQueue->head->next;
				current->next = NULL;
			} else {
				prev->next = current->next;
				current->next = NULL;
			}
			return node;
		}
		i++;
		if (i > 1){
			prev = current;
			current = current->next;
		}else
			current = current->next;
	}
	return node;
}

void displayQ(struct threadQueues *threadQueue){
	struct queue *current;
	current = threadQueue->head;
	if(current == NULL){
	}else{
		while (current != NULL) {
			current = current->next;
		}
	}
}

int removeparentlink(struct threadQueues *threadQueue,myThread *readyThread){
	struct queue *current = threadQueue->head;

	while(current != NULL){
		if(current->readyThread->parent != NULL){
			if(current->readyThread->parent->threadId == readyThread->threadId)
				current->readyThread->parent = NULL;
		}
		current = current->next;
	}
}
