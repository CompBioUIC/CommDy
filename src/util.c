#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "assert.h"
#include "util.h"

int char2color(int ch) {
	if ('0' <= ch && ch <= '9') {
		return ch-'0';
	} else if ('A' <= ch && ch <= 'Z') {
		return ch-'A'+10;
	} else if ('a' <= ch && ch <= 'z') {
		return ch-'a'+10;
	} else if (ch >= 'Z' || ch >= 'z') {
		assertf(1, "Invalid color code '%c'", ch);
	} else {
		return -1;
	}
}

int color2char(int color) {
	if (color < 0) {
		return '?';
	} else if (color < 10) {
		return '0'+color;
	} else if (color-10 <= 'Z'-'A') {
		return 'A' + color-10;
	} else if (color<0) {
		return '?';
	} else {
		assertf(0, "Invalid color %d", color);
	}
}

#ifdef DEBUG_MEMORY_LEAK
void *_CAlloc(size_t count, size_t size, char*file, int line)
#else
void *_CAlloc(size_t count, size_t size)
#endif
{
	void *p = calloc(count, size);
	assertf(p, "Out of memory\n");
#ifdef DEBUG_MEMORY_LEAK
	fprintf(stderr, "%s:%d alloc %p\n", file, line, p);
#endif
	return p;
}

#ifdef DEBUG_MEMORY_LEAK
void *_MAlloc(size_t size, char*file, int line)
#else
void *_MAlloc(size_t size)
#endif
{
	void *p = malloc(size);
	assertf(p, "Out of memory\n");
	//addPointer(p);
#ifdef DEBUG_MEMORY_LEAK
	fprintf(stderr, "%s:%d alloc %p\n", file, line, p);
#endif
	return p;
}

#ifdef DEBUG_MEMORY_LEAK
void *_ReAlloc(void *ptr, size_t size, char*file, int line)
#else
void *_ReAlloc(void *ptr, size_t size)
#endif
{
	void *p = realloc(ptr, size);
	assertf(p, "Out of memory\n");
#ifdef DEBUG_MEMORY_LEAK
	if (ptr!=0) fprintf(stderr, "%s:%d free %p\n", file, line, ptr);
	fprintf(stderr, "%s:%d alloc %p\n", file, line, p);
#endif
	return p;
}

#ifdef DEBUG_MEMORY_LEAK
void _Free(void *ptr, char*file, int line)
#else
void _Free(void *ptr)
#endif
{
	free(ptr);
#ifdef DEBUG_MEMORY_LEAK
	fprintf(stderr, "%s:%d free %p\n", file, line, ptr);
#endif
}

void print_lapsed_time(int t0, int format) {
	int d = time(0)-t0;
	if (format==1) {
		printf("%d:%02d:%02d:%02d", d/3600/24,d/3600%24, d/60%60, d%60);
	} else {
		printf("[%d:%02d:%02d:%02d]", d/3600/24,d/3600%24, d/60%60, d%60);
	}
	//printf("[%dd%2dh%02dm%02ds]", d/3600/24,d/3600%24, d/60%60, d%60);
}

