#ifndef _util_h_
#define _util_h_

#include <stdlib.h>

#define DEFAULT_MAX_COLOR 9

//#define DEBUG_MEMORY_LEAK

int char2color(int ch);
int color2char(int color);

#ifdef DEBUG_MEMORY_LEAK

#define CAlloc(count, size) (_CAlloc(count, size, __FILE__, __LINE__))
#define MAlloc(size) (_MAlloc(size, __FILE__, __LINE__))
#define ReAlloc(ptr, size) (_ReAlloc(ptr, size, __FILE__, __LINE__))
#define Free(ptr) (_Free(ptr, __FILE__, __LINE__))

void *_CAlloc(size_t count, size_t size, char*file, int line);
void *_MAlloc(size_t size, char*file, int line);
void *_ReAlloc(void *ptr, size_t size, char*file, int line);
void _Free(void *ptr, char*file, int line);

#else

#define CAlloc(count, size) (_CAlloc(count, size))
#define MAlloc(size) (_MAlloc(size))
#define ReAlloc(ptr, size) (_ReAlloc(ptr, size))
#define Free(ptr) (_Free(ptr))

void *_CAlloc(size_t count, size_t size);
void *_MAlloc(size_t size);
void *_ReAlloc(void *ptr, size_t size);
void _Free(void *ptr);
#endif

void print_lapsed_time(int t0, int format);

#endif
