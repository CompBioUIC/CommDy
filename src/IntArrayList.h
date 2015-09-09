#ifndef _int_array_list_h_
#define _int_array_list_h_

#define INT_ARRAY_LIST_INIT_SIZE 50
#define INT_ARRAY_LIST_INCREMENT 20

typedef struct {
	int count, max, increment;
	int *p;
} IntArrayList;

void initializeSizeIAL(IntArrayList *a, int init_size, int increment);
void initializeIAL(IntArrayList *a);
void finalizeIAL(IntArrayList *a);

void clearIAL(IntArrayList *a);
void addIAL(IntArrayList *a, int i);
void printIAL(IntArrayList *a);

#endif
