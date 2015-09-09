#include<stdio.h>
#include<string.h>
#include<time.h>
#include<signal.h>

#include"gtmfile.h"
#include"util.h"
#include"assert.h"
#include"PartialColoring.h"
#include"PartialColoringHeap.h"

//#define DEBUG
//#define DEBUG_GROUP_COLOR
//#define DEBUG_IND

static int interrupted=0;
int interrupted_print = 0;
const int num_star_to_skip = 2;
time_t interrupted_t;

static void sigint_handler(int signum) {
	signal(SIGINT, SIG_IGN);
	time_t t;
	time(&t);
	if (interrupted_t==t) {
		interrupted++;
	} else {
		interrupted=1;
		if (interrupted_print) fprintf(stderr, " C-C twice or C-\\ to quit\n");
	}
	signal(SIGINT, sigint_handler);
	time(&interrupted_t);
}

void printIndividualColor(PartialColoring *pc, int ind_count, int max_color) {
	int sum=0;
	for (int i=0;i<ind_count;i++) {
		int min=0x7FFFFFFF, color=-1;
		printf("ind %d:", i);
		for (int c=0;c<=max_color;c++) {
			int cost = getMincost(pc, i, c);
			printf(" %d", cost);
			if (min>cost) {
				min=cost;
				color=c;
			}
		}
		printf(" min %d (%d)\n", min, color);
		sum+=min;
	}
	printf("lb %d\n", sum);
}

void printIndividualColoringCost(PartialColoring *pc, PartialColoring *pc2, int ind_count, int max_color, 
       int lb) {

       //printf("\n");
       //printf("g%d t%d c%c , lb %d\n", 
       //      pc2->group, pc2->timestep, 
       //      color2char(pc2->group_color[pc2->group]),
       //      lb);
       printf("time %2d :", pc2->timestep);

//       int debug_i=0;
       int sum0=0;
       for (int i=0;i<ind_count;i++) {
               //if (i/10!=debug_i) continue;
               sum0+=getMincost(pc2, i, 0);
               for (int c=0;c<=max_color;c++) {
                       printf("%c", color2char(getMincost(pc2, i, c)));
               }
               printf(" ");
       }
       //printf(" sum0 %d", sum0);
       printf(" lb %d", lb);

//     printf("\n===>\n");
//     for (int i=0;i<ind_count;i++) {
//             //if (i/10!=debug_i) continue;
//             for (int c=0;c<=max_color;c++) {
//                     if (getMincost(pc, i, c)==getMincost(pc2, i, c)) {
//                             printf(".");
//                     } else {
//                             printf("%c", color2char(getMincost(pc2, i, c)));
//                     }
//             }
//             printf(" ");
//     }

       printf("\n");
}

int exhaustiveSearch(int switch_cost, int absence_cost, int visit_cost, 
	int g0, linked_group **group_array, int g1, 
	const int *time_lowerbound, int time_count, 
	int subt1, int subt2, 
	const int *group_size, const int *group_time, int ind_count, 
	int *global_min, int *min_group_color, 
	int exact, int max_color, int time_limit, int print_info, int print_strictly_less_than_min) {

	interrupted_print = print_info;

	assert(0<=g0);
	assert(g0<g1);
	//assert(group_array[g1]!=NULL);

	time_t t_init;
	time(&t_init);

	//#ifdef DEBUG_GROUP_COLOR
	//// initialize debug stuff
	//char *debug_gcolor_str = "1212322121122121213221211221";
	//const int debug_gcolor_count = strlen(debug_gcolor_str);
	//int debug_gcolor[debug_gcolor_count];
	//for (int i=0;i<debug_gcolor_count;i++) {
	//	debug_gcolor[i] = char2color(debug_gcolor_str[i]);
	//	assert(debug_gcolor[i]>=0);
	//	assert(debug_gcolor[i]<=max_color);
	//}
	//printf("debug gcolor: ");
	//for (int i=0;i<debug_gcolor_count;i++) {
	//	printf("%c", debug_gcolor_str[i]);
	//}
	//printf("\n");
	//#endif

	#ifdef DEBUG_IND
	int debug_ind=0;
	#endif


	// initialize heap

	PartialColoringHeap heap;
	heapInitialize(&heap);

	// first partial coloring
	PartialColoring *pc = MAlloc(sizeof(PartialColoring));

	initialize(pc, group_array[g0]->group, group_array[g0]->member_count, 
		ind_count, max_color+1, 0);
	pc->timestep = group_array[g0]->timestep;
	pc->group_color[0]=1;
	pc->lowerbound=0;

	{
		int is_member[ind_count];
		memset(is_member, 0, sizeof(int)*ind_count);
		for (int j=0;j<group_size[g0];j++) {
			is_member[group_array[g0]->member[j]]=1;
		}

		// compute lowerbound
		for (int ind=0;ind<ind_count;ind++) { // for each non-member
			if (is_member[ind]) {
//				int ind_lb=visit_cost;
				for (int c=0;c<=max_color;c++) {
					int min=0x7FFFFFFF;
					if (c==pc->group_color[0]) {
						setMincost(pc, ind, c, 0);
						min=0;
//						ind_lb=0;
					} else {
						setMincost(pc, ind, c, visit_cost);
						min=visit_cost;
					}
					#ifdef DEBUG_IND
					if (ind==debug_ind) 
						printf("ind %d time %d color %d cost %d\n", 
								ind, pc->timestep, c, min);
					#endif
				}
//				pc->lowerbound+=ind_lb;
			} else {
//				int ind_lb=absence_cost;
				for (int c=0;c<=max_color;c++) {
					int min=0x7FFFFFFF;
					if (c==pc->group_color[0]) {
						setMincost(pc, ind, c, absence_cost);
						min=absence_cost;
					} else {
						setMincost(pc, ind, c, 0);
						min=0;
//						ind_lb=0;
					}
					#ifdef DEBUG_IND
					if (ind==debug_ind) 
						printf("ind %d time %d color %d cost %d\n", 
								ind, pc->timestep, c, min);
					#endif
				}
//				pc->lowerbound += ind_lb;
			}
		}
		assert(pc->lowerbound==0);
		if (time_lowerbound) {
			pc->lowerbound += time_lowerbound[pc->timestep+1];
		}
	}

	if (print_info>=2) {
		printf("init group color: ");
		printGroupColor(pc->group_color, pc->group+1-g0, group_time+g0, print_info);
		fflush(stdout);
	}

	heapInsert(&heap, pc);

	int subtime_count=subt2-subt1;
	int _time_heap_count[subtime_count];
	memset(_time_heap_count, 0, sizeof(int)*subtime_count);
	int *time_heap_count=_time_heap_count-subt1;
	time_heap_count[pc->timestep]++;

	time_t t0, t1;
	time(&t0);
	t1=t0;
	interrupted_t=t0;
//	const int time_buff_size=20;
//	char time_buff[time_buff_size];

//	printf("%9s", "");
//	for (int i=0;i<ind_count;i++) {
//		char buf[10];
//		snprintf(buf, 10, "%%-%dd", max_color+2);
//		printf(buf, i+1);
//	}
//	printf("\n");

	//printIndividualColoringCost(pc, pc, ind_count, max_color, pc->lowerbound);
	
	//signal(SIGINT, sigint_handler);
    

	// main loop
	while (!heapIsEmpty(&heap)) {

		pc = heapExtractMin(&heap);
		assert(pc);
		time_heap_count[pc->timestep]--;

		#ifdef DEBUG
		if (print_info) {
			printf("extract min: group %d timestep %d lb %d ", 
				pc->group, pc->timestep, pc->lowerbound);
			printGroupColor(pc->group_color, pc->group+1, group_time, print_info);
		}
		#endif

        //if (print_info) {
        //    printf("trace ");
        //    printGroupColor(pc->group_color, pc->group+1-g0, group_time+g0, print_info);
        //    printf(" cost %d min %d\n", pc->lowerbound, *global_min);
        //}

		if (pc->lowerbound<=*global_min) {
			const int g2 = pc->group+1;
			if (g2<g1) {
				// expand partial solution

				int max_valid_color=-1;
				int valid_color[max_color+1];
				memset(valid_color, 0, sizeof(int)*(max_color+1));

				int use_up_color=0;
				#ifdef DEBUG_GROUP_COLOR
				max_valid_color=debug_gcolor[g2];
				valid_color[max_valid_color]=1;
				#else
				// find used colors
				for (int j=0;j<=pc->group-g0;j++) {
					const int c = pc->group_color[j];
					assert(c<=max_color);
					valid_color[c]++;
					if (c>max_valid_color) {
						max_valid_color=c;
					}
				}

				if (exact && g1 - pc->group -1 == max_color - max_valid_color) {
					assert(max_valid_color<max_color);
					use_up_color=1;
				}

				if (max_valid_color<max_color) {
					valid_color[++max_valid_color]=1; // add new color
				}

				// eliminate invalid colors
				valid_color[0]=0; // blank color
				for (int j=pc->group;j>=g0;j--) {
					// should change this to binary search
					if (group_time[j]==group_time[g2]) {
						valid_color[pc->group_color[j-g0]]=0;
					} else {
						break;
					}
				}
				#endif

				#ifdef DEBUG
				// debug info
                if (print_info==1) {
                    printf("valid color (max %d):", max_valid_color);
                    for (int i=0;i<=max_valid_color;i++) {
                        if (valid_color[i]) printf(" %d", i);
                    }
                    printf("\n");
                }
				#endif

				for (int gcolor=(use_up_color?max_valid_color:1);
                        gcolor<=max_valid_color;gcolor++) { // for each valid color
					if (!valid_color[gcolor]) continue;

					// is member of current group g2
					int is_member[ind_count];
					memset(is_member, 0, sizeof(int)*ind_count);
					for (int j=0;j<group_size[g2];j++) {
						is_member[group_array[g2]->member[j]]=1;
					}


					// new partial solution
					PartialColoring *pc2 = MAlloc(sizeof(PartialColoring));
					initialize(pc2, g2, group_size[g2], ind_count, max_color+1, 0);
					pc2->timestep = group_time[g2];
					memcpy(pc2->group_color, pc->group_color, sizeof(int) * (g2-g0));
					memcpy(pc2->mincost, pc->mincost, sizeof(int)*ind_count*(max_color+1));
					pc2->group_color[g2-g0]=gcolor;

					// compute lowerbound
					int lb=0;

					if (pc->timestep == pc2->timestep) { // add beta1 costs

						for (int ind=0;ind<ind_count;ind++) {
							int ind_lb=0x7FFFFFFF;
							if (is_member[ind]) {  // for each member
								for (int c=0;c<=max_color;c++) {
									int min=getMincost(pc, ind, c);
									if (c!=gcolor) min += visit_cost;
									#ifdef DEBUG_IND
									if (ind==debug_ind) printf("visit %d %d\n", c, gcolor);
									#endif
									assert(min>=0);
									assert(min<0x7FFFFFFF);
									setMincost(pc2, ind, c, min);
									if (min<ind_lb) ind_lb=min;
									#ifdef DEBUG_IND
									if (ind==debug_ind) 
										printf("ind %d time %d color %d cost %d\n", 
												ind, pc2->timestep, c, min);
									#endif
								}
							} else { // for each non-member
								for (int c=0;c<=max_color;c++) {
									int cost = getMincost(pc, ind, c);
									if (c==gcolor) {
										cost += absence_cost;
										setMincost(pc2, ind, c, cost);
										#ifdef DEBUG_IND
										if (ind==debug_ind) printf("absence %d\n", c);
										#endif
									}
									if (ind_lb>cost) ind_lb=cost;
									#ifdef DEBUG_IND
									if (ind==debug_ind) 
										printf("ind %d time %d color %d cost %d\n", 
												ind, pc2->timestep, c, cost);
									#endif
								}
							}
							assert(ind_lb<0x7FFFFFFF);
							lb+=ind_lb;
							assert(lb>=0);
						}

					} else { // add beta1 + alpha costs

						for (int ind=0;ind<ind_count;ind++) {
							int ind_lb=0x7FFFFFFF;
							if (is_member[ind]) { // member of g2
								// first group in this timestep, no absense cost for member of g2
								for (int c=0;c<=max_color;c++) {
									int min = 0x7FFFFFFF;
									for (int d=0;d<=max_color;d++) {
										int cost = getMincost(pc, ind, d);
										if (c!=d) {
											cost += switch_cost;
											#ifdef DEBUG_IND
											if (ind==debug_ind) 
												printf("%s:%d switch %d %d\n", 
													__FILE__, __LINE__, d, c);
											#endif
										}
										if (cost<min) min=cost;
									}
									if (c!=gcolor) min += visit_cost;
									#ifdef DEBUG_IND
									if (ind==debug_ind) printf("visit %d %d\n", c, gcolor);
									#endif
									assert(min>=0);
									assert(min<0x7FFFFFFF);
									setMincost(pc2, ind, c, min);
									if (min<ind_lb) ind_lb=min;
									#ifdef DEBUG_IND
									if (ind==debug_ind) 
										printf("ind %d time %d color %d min %d\n", 
												ind, pc2->timestep, c, min);
									#endif
								}
							} else { // non-member of g2
								for (int c=0;c<=max_color;c++) {
									int min = 0x7FFFFFFF;
									for (int d=0;d<=max_color;d++) {
										int cost = getMincost(pc, ind, d);
										if (c!=d) {
											cost += switch_cost;
											#ifdef DEBUG_IND
											if (ind==debug_ind) printf("%s:%d switch %d %d\n", 
												__FILE__, __LINE__, d, c);
											#endif
										}
										if (cost<min) min=cost;
									}
									if (gcolor==c) min += absence_cost;
									#ifdef DEBUG_IND
									if (ind==debug_ind) printf("absence %d\n", c);
									#endif
									assert(min>=0);
									assert(min<0x7FFFFFFF);
									setMincost(pc2, ind, c, min);
									if (min<ind_lb) ind_lb=min;
									#ifdef DEBUG_IND
									if (ind==debug_ind) 
										printf("ind %d time %d color %d min %d\n", 
												ind, pc2->timestep, c, min);
									#endif
								}
							}
							assert(ind_lb<0x7FFFFFFF);
							lb+=ind_lb;
							assert(lb>=0);
						}
					}

                    //#ifdef DEBUG
					//printIndividualColoringCost(pc, pc2, ind_count, max_color, lb);
                    //#endif

					pc2->lowerbound = lb;
					if (time_lowerbound && pc2->timestep+1<time_count) {
						pc2->lowerbound += time_lowerbound[pc2->timestep+1];
					}

					if (pc2->lowerbound <= *global_min) {

						int rc = heapInsert(&heap, pc2);
						if (!rc) {
							printf("Heap is full: size %d\n", heap.item_count);
							printf("min %d\n", *global_min);
							exit(1);
						}

						time_heap_count[pc2->timestep]++;

						if (print_info==2) {
							time(&t1);
#ifndef DEBUG
							if (t1-t0>10 || (t1-t0>1 && interrupted==1)) // print every ... seconds
#endif
							{
								t0=t1;

								print_lapsed_time(t_init, print_info);
								printf(" total heap size %d lb %d min %d", 
                                    heap.item_count, pc2->lowerbound, *global_min);
								printf(" heap_item (");
								for (int t=subt1;t<subt2;t++) {
									printf(" %d", time_heap_count[t]);
								}
								printf(")\n");

								//printf(" insert timestep %d ", pc2->timestep);
								//printGroupColor(pc2->group_color, pc2->group+1-g0, group_time+g0, print_info);

							}
						}

					} else {
						#ifdef DEBUG
						if (print_info>=1) {
							printf(" ignore ");
							printGroupColor(pc2->group_color, pc2->group+1-g0, group_time+g0, print_info);
							printf(" heapsize %d lb %d min %d\n", heap.item_count, pc2->lowerbound, *global_min);
						}
						#endif
						finalize(pc2);
						Free(pc2);
					}

				}

				if (interrupted>1) {
					printf("Interrupted. Terminating ...\n");
					printf(" min %d ", *global_min);

					finalize(pc);
					Free(pc);
					heapFinalize(&heap, __FILE__, __LINE__);

					signal(SIGINT, SIG_DFL);
					return 2;
				}

                time(&t1);
				if (time_limit>0 && t1-t_init > time_limit) {
					if (print_info>=2) {
						printf("\nTime limit exceeded. Cleaning up ...\n");

						print_lapsed_time(t_init, print_info);
						printf(" min %d ", *global_min);
						//if (min_group_color!=0) printGroupColor(min_group_color, g1, group_time, print_info);
                        printf("\n");
					}

					// clean up
					finalize(pc);
					Free(pc);
					heapFinalize(&heap, __FILE__, __LINE__);

					signal(SIGINT, SIG_DFL);

					return 0;
				}
			} else { // found a complete coloring

				if (time_lowerbound && time_lowerbound[0]>pc->lowerbound) {
					printf("Error %s:%d time_lowerbound[0]=%d > cost=%d\n", 
							__FILE__, __LINE__, time_lowerbound[0], pc->lowerbound);
					printf(" lb %d ", pc->lowerbound);
					printGroupColor(pc->group_color, pc->group+1-g0, group_time+g0, print_info);
					exit(1);
				}

				if (pc->lowerbound <= *global_min) {
                    if (!print_strictly_less_than_min || pc->lowerbound < *global_min) {
                        if (print_info==1) {
                            //print_lapsed_time(t_init, print_info);
                            printf("%d : ", pc->lowerbound);
                            printGroupColor(pc->group_color, pc->group+1-g0, group_time+g0, print_info);
                            fflush(stdout);
                        } else if (print_info==2) {
                            print_lapsed_time(t_init, print_info);
                            printf(" min %d ", pc->lowerbound);
                            printGroupColor(pc->group_color, pc->group+1-g0, group_time+g0, print_info);
                            fflush(stdout);
                        }
                    }

					if (pc->lowerbound < *global_min) {
						*global_min = pc->lowerbound;
						if (min_group_color!=0) {
							memcpy(min_group_color, pc->group_color, sizeof(int)*(g1-g0));
						}
					}


					//if (print_info && *global_min ==0) {
					//	printf("Something is wrong. min = 0\n");
					//	assert(*global_min);
					//}

                    if (time_lowerbound && *global_min==time_lowerbound[0]) {
                        //printf("ub %d == lb %d", *global_min, time_lowerbound[0]);
                        break;
                    }
				}

			}

			//#ifdef DEBUG
			//if (print_info) {
			//	printf("\n");
			//}
			//#endif

		} else {
			#ifdef DEBUG
			if (print_info) {
				printf(" ignore ");
				printGroupColor(pc->group_color, pc->group+1-g0, group_time+g0, print_info);
				printf(" heapsize %d lb %d min %d\n", heap.item_count, pc->lowerbound, *global_min);
			}
			#endif

		}

		finalize(pc);
		Free(pc);

	}

	if (print_info>=2) {
        if (heapIsEmpty(&heap)) {
            printf("Heap is empty. Done!\n");
        }

		print_lapsed_time(t_init, print_info);
		printf(" min %d\n", *global_min);
		//if (min_group_color!=0) printGroupColor(min_group_color, g1, group_time, print_info);
	}
	
	heapFinalize(&heap, __FILE__, __LINE__);

	signal(SIGINT, SIG_DFL);

	return 1;
}

#define WIDTH "4"
void print_debug_table(int time_count, char** lb_kind, int** sum_lbs) {
    printf("%2s", "");
    for (int j=0;j<time_count;j++) printf("  %"WIDTH"d", j+1); printf("\n");
    for (int i=0;i<time_count;i++) {
        printf("%2d", i+1);
        for (int j=1;j<=time_count;j++) {
            if (j<=i) {
                // unused lower triangle
                printf("  %"WIDTH"c", ' ' );
            } else if (lb_kind[i][j]=='?') {
                // value has not been computed
                printf("  %"WIDTH"c", lb_kind[i][j]);
            } else if (lb_kind[i][j]=='*') {
                // have valid value. 
                // hit time limit. lb from DP sum
                printf(" +%"WIDTH"d", sum_lbs[i][j]);
            } else if (lb_kind[i][j]=='-') {
                // did not compute ex since it might not terminate in time
                printf("  %"WIDTH"s", "+++");
            } else {
                // got from exhaustive search
                printf("  %"WIDTH"d", sum_lbs[i][j]);
            }
        }
        printf("\n");
    }
}

void compute_interval_lowerbound(gtm_data *gtm, 
		int switch_cost, int absence_cost, int visit_cost, int max_color, int time_limit, 
		int print_info, int** t1t2_lb, int** sum_lbs, int** ex_lbs, char** lb_kind) {

	const int group_count = gtm->group_count;
	const int ind_count = gtm->ind_count;
	const int time_count = gtm->time_count;
	const int max_time_size= gtm->max_time_size;
	const int *time_first_group = gtm->time_first_group;
	const int *group_time = gtm->group_time;
	const int *group_size = gtm->group_size;
	linked_group **group_array = gtm->group_array;

	int *ind_exists = gtm->ind_exists;
	int group_color[group_count];

	// lowerbound of interval [t1, t2)
	int time_lowerbound[time_count+1];

	// init
	for (int i=0;i<time_count;i++) {
		for (int j=0;j<=time_count;j++) {
			if (i==j || j-i==1) {
				t1t2_lb[i][j]=0;
				ex_lbs[i][j]=0;
				sum_lbs[i][j]=0;
				lb_kind[i][j]='=';
			} else {
				t1t2_lb[i][j]=0x7FFFFFFF;
				ex_lbs[i][j]=0x7FFFFFFF;
				sum_lbs[i][j]=0;
				lb_kind[i][j]='?';
			}
		}
	}

	int so_long=0;
	for (int wsize=2;wsize<=time_count;wsize++) {
		int all_toolong=1;
		for (int t1=0;t1<=time_count-wsize;t1++) {
			int t2=t1+wsize;
			int g1=time_first_group[t1], g2=time_first_group[t2];

			if (print_info==1) {
				printf("wsize %d ", wsize);
				printf("time %d:%d (group %d:%d) ", t1+1, t2, g1+1, g2+1);
				fflush(stdout);
			}

			// lowerbound from DP table
			int num_star=0;
			{
				int max=0;
				for (int d=1;d<wsize;d++) {
                    int lb2 = t1t2_lb[t1][t1+d] + t1t2_lb[t1+d][t2];
					if (lb2>max) max=lb2;
					if (lb_kind[t1][t1+d]=='*' || lb_kind[t1+d][t2]=='*') {
						num_star++;
					}
				}
				sum_lbs[t1][t2]=max;
			}

			// lowerbound from exhaustive search
			int rc=0;
			if (num_star<num_star_to_skip && !so_long) {
				if (g1>=g2) { // in case that empty timesteps has g2=0
					rc=1;
					ex_lbs[t1][t2] = 0;
				} else {
					memset(time_lowerbound, 0, (time_count+1) * sizeof(int));

					ex_lbs[t1][t2] = 0x7FFFFFFF;

					//printf("(tlb");
					for (int t=t1+1;t<=t2;t++) {
						time_lowerbound[t] = t1t2_lb[t][t2];
						//printf(" %d", time_lowerbound[t]);
					}
					time_lowerbound[t1] = sum_lbs[t1][t2];
					//printf(") "); fflush(stdout);

					rc = exhaustiveSearch(switch_cost, absence_cost, visit_cost,
							g1, group_array, g2, time_lowerbound, time_count, 
							t1, t2, 
							group_size, group_time, ind_count, &ex_lbs[t1][t2], 0, 
							0, max_color<g2-g1?max_color:g2-g1, time_limit, 0, 1);
				}
			} else {
				ex_lbs[t1][t2] = 0;
			}

			if (rc==1) { // exhaustive search terminated in time limit
				all_toolong=0;
				if (print_info==1) printf("lb %d\n", ex_lbs[t1][t2]);

				assertf(ex_lbs[t1][t2]>=sum_lbs[t1][t2], "%d < %d", ex_lbs[t1][t2], sum_lbs[t1][t2]);

				if (ex_lbs[t1][t2]<sum_lbs[t1][t2]) {
                    assertf(0, "Impossible");
				} else if (ex_lbs[t1][t2]>sum_lbs[t1][t2]) {
					lb_kind[t1][t2]='>';
				} else {
					lb_kind[t1][t2]='=';
				}
                t1t2_lb[t1][t2] = ex_lbs[t1][t2];

			} else { // otherwise, use lowerbound from DP
				if (print_info==1) printf("lb *\n");
                if (num_star<num_star_to_skip) {
                    lb_kind[t1][t2]='*';
                } else {
                    lb_kind[t1][t2]='-';
                }
				t1t2_lb[t1][t2] = sum_lbs[t1][t2];
			}

			if (rc==2) { // interrupted by user
				so_long=1;
			}

            if (0) { // debug
                print_debug_table(time_count, lb_kind, sum_lbs);
            } else if (print_info==2) {
		        if (t1==0) printf("w %2d:", wsize);
                char lb_type=lb_kind[t1][t2];
                if (lb_type=='>' || lb_type=='=') {
                    lb_type='=';
                } else if (lb_type=='*' || lb_type=='-') {
                    lb_type='+';
                } else {
                    lb_type=' ';
                }
                printf(" %3d%c", t1t2_lb[t1][t2], lb_type);
		        if (t1==time_count-wsize) printf("\n");
                fflush(stdout);
            }

		}
		so_long=all_toolong;
	}

}

