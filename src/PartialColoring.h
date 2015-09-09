#ifndef _PartialColoring_h_
#define _PartialColoring_h_

typedef struct PartialColoringSt {
	int group, timestep, lowerbound;
	int member_count, ind_count, color_count;
	int *mincost;
	int *group_color;
} PartialColoring;

void initialize(PartialColoring *pc, int group, int member_count, int ind_count, 
	int color_count, int lowerbound);
void finalize(PartialColoring *pc);

int getMincost(PartialColoring *pc, int member, int color);
int setMincost(PartialColoring *pc, int member, int color, int cost);

#endif
