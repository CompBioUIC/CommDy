#ifndef _assert_h_
#define _assert_h_

#include<stdlib.h>

#define assert(expr) \
	if(!(expr)) { fprintf(stderr, "ERROR %s:%d assertion "#expr" violated.\n", __FILE__, __LINE__); exit(1); } 

#define assertf(expr, format, args ...) \
	if(!(expr)) { fprintf(stderr, "ERROR %s:%d "format"\n",__FILE__, __LINE__, ##args); exit(1); }

#define warn_if(expr, format, args ...) \
	if(expr) { fprintf(stderr, "ERROR %s:%d "format"\n",__FILE__, __LINE__, ##args); }

#endif
