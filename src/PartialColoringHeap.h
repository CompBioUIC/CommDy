#ifndef _PartialColoringHeap_h_
#define _PartialColoringHeap_h_

#include "PartialColoring.h"

#define HEAP_MAX_SIZE 1000000

#define HEAP_TIME_INCREMENT 8
#define HEAP_ARRAY_INCREMENT 1024

/* array heap */

typedef int (*PcComparer)(PartialColoring *a, PartialColoring *b);

typedef struct PartialColoringHeapSt {
	int item_count;
	int time_increment, array_increment;
	int time_count, alloc_time_count;
	int *size, *alloc_size;
	PartialColoring ***arrays;
} PartialColoringHeap;

void heapInitialize(PartialColoringHeap *heap);
void heapInitializeWithIncrements(PartialColoringHeap *heap, int time_increment, int array_increment);
void heapClear(PartialColoringHeap *heap, char* f, int l);
void heapFinalize(PartialColoringHeap *heap, char *f, int l);
int heapRetainElements(PartialColoringHeap *heap, int cost_upperbound);

//void heapifyUp(PartialColoringHeap *heap);
//void heapifyDown(PartialColoringHeap *heap);
int heapInsert(PartialColoringHeap *heap, PartialColoring *pc);
PartialColoring* heapPeak(PartialColoringHeap *heap);
PartialColoring* heapExtractMin(PartialColoringHeap *heap);
int heapIsEmpty(PartialColoringHeap *heap);
void heapPrint(PartialColoringHeap *heap);

// comparers for changing the behavior of the heaps
int partialColoringBfs(PartialColoring *a, PartialColoring *b);
int partialColoringDfsBfsHybrid(PartialColoring *a, PartialColoring *b);
int partialColoringDfs(PartialColoring *a, PartialColoring *b);


#endif

