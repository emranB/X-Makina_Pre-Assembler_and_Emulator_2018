#pragma once


/*
	Structure of Queue
	- It has a size
	- It has a pointer to the top Node in Queue
	- It has a pointer to the bottom Node in Queue
*/
typedef struct Queue {
	unsigned int Size;
	struct Node* Top;
	struct Node* Bottom;
} Queue;


/*
Structure of each Node in Queue
	- An Index to determine its LRU algorithm
	- It has a pointer to the next Node in Queue
	- It has a pointer to the prev Node in Queue
*/
typedef struct Node {
	unsigned char LRU_Index;			/* 0 to 31 */
	struct Node* Next;
	struct Node* Prev;
	struct CACHE_LINE CacheContents;	/* Point to a Cache Line */
} Node;


/* Creates and returns a new Queue of specified size */
Queue* CreateQueue(unsigned int);

/* Free Queue Memory */
void FreeQueue(Queue*);

/* Pushes a New NODE to TOP of Cache Queue and return Index of Queued Element */
int PushToQueue(Queue*, unsigned short, signed short*);

/* Remove NODE from a Queue at specified Index */
void RemoveNode(Queue*, unsigned int);

/* Check if an address exists in a Queue */
int IsInQueue(Queue*, signed short);