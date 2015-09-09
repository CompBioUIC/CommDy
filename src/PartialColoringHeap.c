#include<stdio.h>
#include<string.h>
#include"util.h"
#include"assert.h"

#include"PartialColoring.h"
#include"PartialColoringHeap.h"

/* comparison function */

int partialColoringBfs(PartialColoring *a, PartialColoring *b) {
	if (a->timestep == b->timestep) {
		return a->lowerbound < b->lowerbound;
	} else {
		return a->timestep < b->timestep;
	}
}

int partialColoringLowerbound(PartialColoring *a, PartialColoring *b) {
	return a->lowerbound < b->lowerbound;
}

int partialColoringDfsBfsHybrid(PartialColoring *a, PartialColoring *b) {
	const int w_size = 4;
	if (a->timestep/w_size == b->timestep/w_size) {
		return a->lowerbound < b->lowerbound;
	} else {
		return a->timestep > b->timestep;
	}
}

int partialColoringDfs(PartialColoring *a, PartialColoring *b) {
	if (a->group == b->group) {
		if (a->lowerbound == b->lowerbound) {
			return a->color_count < b->color_count;
		} else {
			return a->lowerbound < b->lowerbound;
		}
	} else {
		return a->group > b->group;
	}
}

static PcComparer comparer;

/* array heap */

void heapInitializeWithComparer(PartialColoringHeap *heap, PcComparer _comparer) {
	comparer = _comparer;
	heapInitializeWithIncrements(heap, HEAP_TIME_INCREMENT, HEAP_ARRAY_INCREMENT);
}

void heapInitialize(PartialColoringHeap *heap) {
	heapInitializeWithComparer(heap, &partialColoringDfs);
}

void heapInitializeWithIncrements(PartialColoringHeap *heap, int time_increment, int array_increment) {
	heap->item_count=0;
	heap->time_increment=time_increment;
	heap->array_increment=array_increment;

	heap->time_count=0;
	heap->alloc_time_count=0;
	heap->size = 0;
	heap->alloc_size = 0;
	heap->arrays = 0;
}

void heapClear(PartialColoringHeap *heap, char* f, int l) {
//	fprintf(stderr, "%s:%d free all %d items\n", f, l, heap->item_count);
//	heapPrint(heap);
	for (int t=0;t<heap->alloc_time_count;t++) {
		PartialColoring** p = heap->arrays[t];
//		printf("free time %d\n", t);
		for (int i=0;i<heap->size[t];i++) {
//			printf("	free pc %d\n", i);
			finalize(p[i]);
			Free(p[i]);
			heap->item_count--;
			assert(heap->item_count>=0);
		}
		heap->alloc_size[t]=0;
		heap->size[t]=0;
		heap->arrays[t]=0;
		Free(p);
	}
}

void heapFinalize(PartialColoringHeap *heap, char* f, int l) {
	heapClear(heap, f, l);
	int count = heap->item_count;
	if (count!=0) {
		printf("Error in finalizing heap: %d items left\n", count);
		assert(count!=0);
	}
	Free(heap->arrays);
	Free(heap->size);
	Free(heap->alloc_size);
}

void heapifyUpFrom(PartialColoring **array, int size, int i) {
	while (i>0) {
		int j = (i+1)/2-1;
		if ((*comparer)(array[i], array[j])) {
			PartialColoring *tmp = array[i];
			array[i] = array[j];
			array[j] = tmp;
			i=j;
		} else {
			break;
		}
	}
}

void heapifyUp(PartialColoring **array, int size) {
	heapifyUpFrom(array, size, size-1);
}

void heapifyDownFrom(PartialColoring **array, int size, int i) {
	while (i<size) {
		int j=2*(i+1);
		if (size<j) {
			break;
		} else if (size==j) {
			j--;
		} else if ((*comparer)(array[j-1], array[j])) {
			j--;
		}
		if ((*comparer)(array[j], array[i])) {
			PartialColoring *tmp = array[i];
			array[i] = array[j];
			array[j] = tmp;
			i=j;
		} else {
			break;
		}
	}
}

void heapifyDown(PartialColoring **array, int size) {
	heapifyDownFrom(array, size, 0);
}

void dummy() {}

int heapInsert(PartialColoringHeap *heap, PartialColoring *pc) {
	const int timestep = pc->timestep;

	// create new heaps for new time step
	while (timestep >= heap->alloc_time_count) {
		int newsize = heap->alloc_time_count + heap->time_increment;

		heap->size = ReAlloc(heap->size, newsize*sizeof(int));
		heap->alloc_size = ReAlloc(heap->alloc_size, newsize*sizeof(int));
		heap->arrays = ReAlloc(heap->arrays, newsize*sizeof(PartialColoring**));

		memset(heap->size + heap->alloc_time_count, 0, heap->time_increment*sizeof(int));
		memset(heap->alloc_size + heap->alloc_time_count, 0, heap->time_increment*sizeof(int));
		memset(heap->arrays + heap->alloc_time_count, 0, heap->time_increment*sizeof(PartialColoring **));

		heap->alloc_time_count = newsize;

	}

    //printf("time: item %d heap %d\n", pc->timestep, heap->time_count);

	if (timestep+1>heap->time_count) {
		heap->time_count = timestep+1;
	}

	// increase heap size for new item
	if (heap->size[timestep] >= heap->alloc_size[timestep]) {
		int newsize = heap->alloc_size[timestep] + heap->array_increment;

		heap->arrays[timestep] = ReAlloc(heap->arrays[timestep], newsize * sizeof(PartialColoring*));
		memset(heap->arrays[timestep] + heap->alloc_size[timestep], 0, heap->array_increment*sizeof(PartialColoring **));
		heap->alloc_size[timestep] = newsize;
	}

	heap->arrays[timestep][heap->size[timestep]++] = pc;
	heapifyUp(heap->arrays[timestep], heap->size[timestep]);

	int found=0;
	for (int i=0;i<heap->size[timestep];i++) {
		if (heap->arrays[timestep][i]==pc) { found=1; break; }
	}
	assert(found);

	heap->item_count++;
	assert(heap->item_count>=0);

	return 1;
}

PartialColoring* heapPeak(PartialColoringHeap *heap) {
    const int last_time = heap->time_count-1;
    PartialColoring **array = heap->arrays[last_time];
    return array[0];
}

PartialColoring* heapExtractMin(PartialColoringHeap *heap) {
	if (comparer == &partialColoringBfs) { 
		int time=0;
		while (heap->size[time]==0 && time<heap->alloc_time_count) time++;
		if (time == heap->alloc_time_count) return 0;
		PartialColoring **array = heap->arrays[time];
		int size = heap->size[time];
		PartialColoring *rc = array[0];
		array[0] = array[--size];
		array[size]=0;
		heapifyDown(array, size);
		heap->size[time] = size;

		heap->item_count--;
		assert(heap->item_count>=0);

		return rc;
	} else {
		while (heap->time_count >0) {
			const int last_time = heap->time_count-1;
			for (int t=last_time+1;t<heap->alloc_time_count;t++) {
				assert(heap->size[t]==0);
			}
			if (heap->size[last_time] > 0) {
				PartialColoring **array = heap->arrays[last_time];
				int size = heap->size[last_time];
				PartialColoring *rc = array[0];
				array[0] = array[--size];
				array[size]=0;
				heapifyDown(array, size);
				heap->size[last_time] = size;

				heap->item_count--;
				assert(heap->item_count>=0);

				return rc;
			}
			heap->time_count--;
		}
	}
	//heap->item_count--;
	//assert(heap->item_count>=0);

	return 0;
}

int heapIsEmpty(PartialColoringHeap *heap) {
	while (heap->time_count>0) {
		const int last_time = heap->time_count-1;
		if (heap->size[last_time] > 0) {
			return 0;
		}
		heap->time_count--;
	}
	return 1;
}

void heapPrint(PartialColoringHeap *heap) {
	printf("heap (%d items):\n", heap->item_count);
	for (int t=0;t<heap->alloc_time_count;t++) {
		printf("time %d (%d items): ", t, heap->size[t]);
		for (int i=0;i<heap->size[t];i++) {
			if (i>0) printf(" ");
			PartialColoring *pc = heap->arrays[t][i];
			for (int j=0;j<=pc->group;j++) {
				printf(" %c", color2char(pc->group_color[j]));
			}
			printf("(%d)", pc->lowerbound);
		}
		printf("\n");
	}
}

int heapRetainElements(PartialColoringHeap *heap, int cost_upperbound) {
}
