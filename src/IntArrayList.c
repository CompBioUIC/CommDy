#include <stdio.h>

#include "util.h"
#include "assert.h"
#include "IntArrayList.h"

void initializeSizeIAL(IntArrayList *a, int init_size, int increment) {
	a->count=0;
	a->max=init_size;
	a->increment=increment;
	a->p = (int*) MAlloc(sizeof(int*)*init_size);
}

void initializeIAL(IntArrayList *a) {
	initializeSizeIAL(a, INT_ARRAY_LIST_INIT_SIZE, INT_ARRAY_LIST_INCREMENT);
}

void finalizeIAL(IntArrayList *a) {
	Free(a->p);
}

void clearIAL(IntArrayList *a) {
	a->count=0;
}

void addIAL(IntArrayList *a, int i) {
	if (a->count==a->max) {
		//printf("increase max size from %d to %d\n", a->max, a->max+a->increment);
		a->max+=a->increment;

		a->p = ReAlloc(a->p, sizeof(int) * a->max);
		// the same thing
		//int *newp = MAlloc(sizeof(int*) * a->max);
		//memcpy(newp, a->p, sizeof(int*) * a->max);
		//Free(a->p);
		//a->p = newp;

		assert(a->p);
	}
	assertf(a->count < a->max, "count %d max %d", a->count, a->max);
	a->p[a->count++] = i;
}

void printIAL(IntArrayList *a) {
	for (int i=0;i<a->count;i++) {
		if (i>0) {
			printf(" %d", a->p[i]);
		} else {
			printf("%d", a->p[i]);
		}
	}
}
