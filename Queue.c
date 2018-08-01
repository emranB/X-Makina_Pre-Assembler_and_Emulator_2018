/*
Author: Emran Billah (em843554@dal.ca)
	Emulation of Cache Queues
*/
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "cpu.h"
#include "Queue.h"



/*
	Maximum LRU Index for Cache Organization Types:
	31 -> Direct Mapped
	31 -> Associative
	4  -> Hybrid
*/
int MAX_SIZE_BY_ORG[3] = { 32, 32, 4 };

/* Properties of Cache being used (set by the Debugger) */
extern struct CACHE_PROPERTIES CacheProperties;

/* 32 Cache Lines */
extern struct CACHE_LINE CACHE[MAX_CACHE_SIZE];


/* 
	Creates and returns a new Queue of specified size 
*/
Queue* CreateQueue(unsigned int size) {
	Queue* queue = (Queue*)malloc(size * sizeof(Queue));

	/* Set max size of Queue */
	queue->Size = size;

	/* Set top and bottom to point to NULL */
	queue->Top = queue->Bottom = NULL;

	return queue;
}



/*
	Free Queue Memory
*/
void FreeQueue(Queue* queue) {
	free(queue);
}


/*
	Pushes a New NODE to TOP of Cache Queue, 
	pointing to a specified Cache Line.
	- Linear search every NODE in Queue to match addr
	- If there is a match
		- Remove NODE from queue
		- Shift all NODEs before it Down by 1
		- Place the removed node at TOP of Queue
	- Else (if there is NO match)
		- Shift all NODEs Down by 1
		- Place new NODE at the TOP of the queue
		- If size of Queue exceeds it max Size
			- Remove last Node from Queue
	- Return Index of Queued Element 
*/
int PushToQueue(Queue* queue, unsigned short address, signed short* data) {
	Node* node = queue->Top;
	Node* temp = NULL;
	unsigned int count;
	while (node->CacheContents.Data) {	/* While the Node has some data */
		if (node->CacheContents.Address == address) { /* Cache HIT */
			node = temp;
		}
		node = node->Next;
		count++;
	}

	RemoveNode(queue, count);

	/* Cache HIT */
	if (temp != NULL) {

	}
}



/*
	- If Dirty Bit is set
		- Write back to Memory
	- Remove NODE from a Queue at specified Index
*/
void RemoveNode(Queue* queue, unsigned int count) {
	Node* node = queue->Top;
	unsigned int i = 0;
	/* Note: Count is Incremented because 
			 we want 'node' to point to the Node at 'count' */
	while (i != (count + 1)) {
		node = node->Next;
		i++;
	}

	/* If Dirty Bit is set, Retain Cache Consistency */
	if (node->CacheLinePtr->DirtyBit)
		RetainCacheConsistency(node->CacheLinePtr->Address, node->CacheLinePtr->Data, WORD, count);

	/* Remove Node from Queue */
	node->Next->Prev = node->Prev;
	node->Prev->Next = node->Next;
	node->Next = node->Prev = NULL;
	queue->Size--;
	free(node);
}






/*
	Check if an address exists in a Queue
	@return count unsigned int -> Index of Node in Queue or NULL
*/
int IsInQueue(Queue* queue, signed short address) {
	unsigned int match = FALSE;
	Node* node         = queue->Top;
	unsigned int count = 0;

	while (node != NULL) {
		if (node->CacheLinePtr->Address == address)
			return count;
		node = node->Next;
		count++;
	}

	return NULL;
}