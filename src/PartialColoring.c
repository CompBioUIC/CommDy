#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"assert.h"
#include"util.h"
#include"PartialColoring.h"

void checkArrayIndexBound(int index, int length) {
	if (index >= length) {
		fprintf(stderr, "%s:%d Array index %d out of bound %d\n", 
				__FILE__, __LINE__, index, length);
		exit(0);
	}
}

void initialize(PartialColoring *pc, int group, int member_count, int ind_count, 
	int color_count, int lowerbound) {
	pc->group=group;
	pc->member_count=member_count;
	pc->ind_count=ind_count;
	pc->color_count=color_count;
	pc->lowerbound = lowerbound;

	pc->mincost = CAlloc(ind_count*color_count, sizeof(int));

	pc->group_color = MAlloc(sizeof(int) * (group+1));
	memset(pc->group_color, 0, sizeof(int)*(group+1));

}

void finalize(PartialColoring *pc) {
	Free(pc->mincost);
	Free(pc->group_color);
}

int getMincost(PartialColoring *pc, int member, int color) {
	checkArrayIndexBound(member*pc->color_count + color, pc->ind_count*pc->color_count);
	if (member*pc->color_count + color < pc->ind_count*pc->color_count) {
		return *(pc->mincost + member*pc->color_count + color);
	}
	return 0x7FFFFFFF;
//	fprintf(stderr, "%s:%d Array index %d out of bound %d\n", 
//		__FILE__, __LINE__, member*pc->color_count + color, pc->ind_count*pc->color_count);
//	fprintf(stderr, "group %d group_size %d member %d color %d\n", 
//		pc->group, pc->ind_count, member, color);
//	exit(0);
}

int setMincost(PartialColoring *pc, int member, int color, int cost) {
	assert(cost>=0);
	assert(cost<0x7FFFFFFF);
	checkArrayIndexBound(member*pc->color_count + color, pc->ind_count*pc->color_count);
	if (member*pc->color_count + color < pc->ind_count*pc->color_count) {
		return *(pc->mincost + member*pc->color_count + color) = cost;
	}
	fprintf(stderr, "%s:%d Array index %d out of bound %d\n", 
		__FILE__, __LINE__, member*pc->color_count + color, pc->ind_count*pc->color_count);
	fprintf(stderr, "group %d group_size %d member %d color %d\n", 
		pc->group, pc->ind_count, member, color);
	exit(0);
}

