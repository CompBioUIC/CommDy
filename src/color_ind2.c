#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include<time.h>

#include"gtmfile.h"
#include"assert.h"
#include"util.h"

//#define COLOR_LOOP_OPT_LV 0	// all colors
//#define COLOR_LOOP_OPT_LV 1	// only visited colors, use flags
#define COLOR_LOOP_OPT_LV 2		// only visited colors, use look-up array

typedef struct Coloring_st{
	int *colors;
	struct Coloring_st *next;
} Coloring;
typedef Coloring* GIColoring;

#define COLOR_DEFAULT 0
#define COLOR_1CHAR 1
#define COLOR_SEP 2

int quiet = 0;
int color_type=COLOR_DEFAULT;
int out_color_type=COLOR_SEP;
int glimit=1, ilimit=1;

int switch_cost=1, absence_cost=1, visit_cost=1;

void print_usage(char *prog_name) {
	printf("usage: %s [<options>] <gtm file>\n", prog_name);
	printf("	-cost : specify cost tuple (i, a, g)\n");
	printf("	-q : print cost instead of colorings\n");
	printf("\n");
	exit(0);
}

int read_group_color(gtm_data *gtm, int *group_color) {
	const int ind_count = gtm->ind_count;
	const int time_count = gtm->time_count;
	const int group_count = gtm->group_count;
	const int *time_first_group = gtm->time_first_group;

	int ch=0;
	int gindex=0;
	int color_count=-1;
	if (color_type == COLOR_1CHAR) {
		//fprintf(stderr, "1char\n");
		while ((ch=getchar())!=EOF) {
			if (ch=='\n') break;
			if (isspace(ch)) continue;

			if (gindex==group_count) {
				fprintf(stderr, "ERROR %s:%d GTM file has %d groups, "
					"more than the input color which has %d\n", 
					__FILE__, __LINE__, group_count, gindex);
			}

			int color=char2color(ch);
			//printf("%c to %d\n", ch, color);
			if (color<0) {
				fprintf(stderr, "Cannot convert '%c' to color\n", ch);
				exit(1);
			}
			group_color[gindex++]=color;
			if (color_count<color) {
				color_count=color;
			}
		}
	} else if (color_type == COLOR_SEP) {
		//fprintf(stderr, "sep\n");
		int color=0;
		int is_defined=0;
		while ((ch=getchar())!=EOF) {
			if (ch=='\n') {
				if (is_defined) {
					if (color_count<color) {
						color_count=color;
					}
					group_color[gindex++]=color;
					//printf("%d\n", color);
					color=0;
					is_defined=0;
				}
				break;
			}
			if (isspace(ch)) {
				if (is_defined) {
					if (color_count<color) {
						color_count=color;
					}
					group_color[gindex++]=color;
					//printf("%d\n", color);
					color=0;
					is_defined=0;
				}
				continue;
			}

			if (gindex==group_count) {
				break;
				fprintf(stderr, "ERROR %s:%d GTM file has %d groups, "
					"more than the input color which has %d\n", 
					__FILE__, __LINE__, group_count, gindex);
			}

			int num=char2color(ch);
			if (num<0) {
				fprintf(stderr, "Cannot convert '%c' to color\n", ch);
				exit(1);
			}
			color = color*10+num;
			is_defined=1;
		}
	} else {
		assertf(0, "Unknown input format %d\n", color_type);
		exit(1);
	}

	if (ch==EOF) return -1;
	if (gindex==0) return 0;

	assertf(gindex==group_count, "GTM file has %d groups while the input color has %d\n", 
				group_count, gindex);

	// check validity
	for (int t=0;t<time_count;t++) {
		const int last = time_first_group[t+1];
		for (int g1=time_first_group[t];g1<last;g1++) {
			for (int g2=g1+1;g2<last;g2++) {
				assertf(group_color[g1]!=group_color[g2],
					"Invalid color: groups %d and %d have same color at time %d\n",
						g1+1, g2+1, t+1);
			}
		}
	}
	
	return color_count+1;
}

void print_output_debug(int ind_count, int time_count, int color_count, 
		int *ind_time_color_min, int *ind_exist) {

	for (int t=0;t<time_count;t++) {
		const int coffset = t*color_count;
		printf("time %2d : ", t);
		int sum=0;
		for (int i=0;i<ind_count;i++) {
			const int ioffset = i*time_count*color_count + coffset;
			if (!ind_exist[i]) continue;
			int ind_min=0x7FFFFFFF;
			for (int c=0;c<color_count;c++) {
				int cost = ind_time_color_min[ioffset + c];
				if (out_color_type==COLOR_1CHAR) {
					printf("%c", color2char(cost));
				} else if (out_color_type==COLOR_SEP) {
					printf(" %d", cost);
				} else {
					assert(0);
				}
				if (cost<ind_min) ind_min=cost;
			}
			printf(" ");
			sum+= ind_min;
		}
		printf(" lb %d\n", sum);
	}

}

void print_output_plain(int ind_count, int time_count, int color_count, 
		int *ind_time_color_min, int *ind_exist, 
		int *ind_time_color_prev, int *ind_time_color_prev_count) {

	for (int i=0;i<ind_count;i++) {
		if (!ind_exist[i]) continue;

		const int ioffset = i*time_count*color_count;

		printf("i %d\n", i+1);

		int icolor[time_count];
		int icolors_count[time_count];
		int icolors[time_count][color_count];

		// find min
		int imin=0x7FFFFFFF;
		for (int c=0;c<color_count;c++) {
			int tmp = ind_time_color_min[ioffset+color_count*(time_count-1)+c];
			if (imin>tmp) imin=tmp;
			assert(imin<0x7FFFFFFF);
			assert(imin>=0);
		}

		// find min colors
		icolors_count[time_count-1]=0;
		for (int c=0;c<color_count;c++) {
			int tmp = ind_time_color_min[ioffset+color_count*(time_count-1)+c];
			if (imin!=tmp) continue;
			icolors[time_count-1][icolors_count[time_count-1]++]=c;
		}

		// enumerate
		int k=time_count-1;
		while (k<time_count) {
			while (icolors_count[k]>0) {
				icolor[k] = icolors[k][--icolors_count[k]];
				if (k==0) {
					for (int t=0;t<time_count;t++) {
						if (out_color_type==COLOR_1CHAR) {
							printf("%c", color2char(icolor[t]));
						} else if (out_color_type==COLOR_SEP) {
							printf(" %d", icolor[t]);
						} else {
							assert(0);
						}
					}
					printf("\n");
				} else {
					// find min colors
					icolors_count[k-1]=0;
					for (int c=0;c<ind_time_color_prev_count[ioffset+k*color_count+icolor[k]];c++) {
						int tmp = ind_time_color_prev[ ioffset*color_count + k*color_count*color_count
							+ icolor[k]*color_count + c];
						icolors[k-1][icolors_count[k-1]++]=tmp;
					}
					assert(icolors_count[k-1]>0);
					k--;
				}
			}
			k++;
		}
		printf("\n");
	}

}

void print_group_color(int *group_color, int group_count) {
	if (out_color_type==COLOR_1CHAR) {
		printf("g ");
	} else if (out_color_type==COLOR_SEP) {
		printf("g");
	} else {
		assertf(0, "Unknown out_color_type %d", out_color_type);
	}
	for (int g=0;g<group_count;g++) {
		if (out_color_type==COLOR_1CHAR) {
			printf("%c", color2char(group_color[g]));
		} else if (out_color_type==COLOR_SEP) {
			printf(" %d", group_color[g]);
		} else {
			assert(0);
		}
	}
	printf("\n");
}

void process_group_color_limit1(gtm_data *gtm, int *group_color, int group_count, 
		int color_count, int **ind_time_group) {

	const int ind_count = gtm->ind_count;
	const int time_count = gtm->time_count;
	const int *group_time = gtm->group_time;
	const int *time_first_group = gtm->time_first_group;

	int t_gcolor_exist[color_count];

	for (int g=group_count-1;g>=0;g--) {
		int c = group_color[g];
		int t = group_time[g];
	}

	// initialization
	int ind_exist[ind_count];
	memset(ind_exist, 0, sizeof(int)*ind_count);
	for (int i=0;i<ind_count;i++) {
		for (int t=0;t<time_count;t++) {
			if (ind_time_group[i][t]<0) continue;
			ind_exist[i]++;
		}
	}

	// print group color
	
	int space_sep=0;

	for (int g=0;g<group_count;g++) {
		if (out_color_type==COLOR_1CHAR) {
			printf("%c", color2char(group_color[g]));
		} else if (out_color_type==COLOR_SEP) {
			if (g>0) printf(" ");
			printf("%d", group_color[g]);
		} else {
			assert(0);
		}
	}

	int time_color_min[2][color_count];
	int icolor[time_count];
	int *curr_time_color_min=&time_color_min[0][0];
	int *prev_time_color_min=&time_color_min[1][0];

	int *time_color_prev[time_count];
	for (int t=0;t<time_count;t++) {
		time_color_prev[t] = MAlloc(sizeof(int)*color_count);
	}

	int total_cost=0;
	time_t t0, t1;
	time(&t0);
	t1=t0;

	const int n_second_count=5;
	int n_second_index=0;
	int n_second[n_second_count];
	memset(n_second, 0, sizeof(int)*n_second_count);

	for (int i=0;i<ind_count;i++) {
		if (!ind_exist[i]) continue;

		//time(&t1);
		//if (t1-t0>5) {
		//	t0=t1;
		//	fprintf(stderr, "%.2f%% completed\n", i*100.0/ind_count);
		//}
		//

#if  COLOR_LOOP_OPT_LV == 1 || COLOR_LOOP_OPT_LV == 2 
		int is_visited_color[color_count];
		int i_color_count=0;
		memset(is_visited_color, 0, sizeof(int)*color_count);
		is_visited_color[0]=1;
		for (int t=0;t<time_count;t++) {
			int g = ind_time_group[i][t];
			if (g<0) continue;
			int gc = (g<0?-1:group_color[ind_time_group[i][t]]);
			assert(gc>=0);
			if (is_visited_color[gc]==0) i_color_count++;
			is_visited_color[gc]++;
		}
#	if COLOR_LOOP_OPT_LV==2
		int i_color[i_color_count];
		i_color_count=0;
		for (int c=0;c<color_count;c++) {
			if (is_visited_color[c])
				i_color[i_color_count++]=c;
		}
#	endif
#else
		int i_color_count=color_count;
#endif

		// dynamic programming
		for (int t=0;t<time_count;t++) {

			// print every ... seconds
			time(&t1);
			if (t1-t0>5) {
				// estimate time to go
				n_second[n_second_index] = i*time_count+t;
				int n_second_prev = n_second_index;
				n_second_index = (n_second_index +1) %n_second_count;
				float rate = (n_second[n_second_prev] - n_second[n_second_index])
					/5./n_second_count;

				float remaining=(ind_count*time_count-n_second[n_second_prev])/rate;
				char *unit;
				if (remaining<60) {
					unit="sec";
				} else {
					remaining/=60;
					if (remaining<60) {
						unit="min";
					} else {
						remaining/=60;
						if (remaining<24) {
							unit="hour";
						} else {
							remaining/=24;
							unit="day";
						}
					}
				}
				fprintf(stderr, "ind %d/%d time %d/%d #colors %d "
						"min %d remaining %.2f %s\n", 
						i+1, ind_count, t+1, time_count, 
						i_color_count, total_cost, 
						remaining, unit);
				t0=t1;
			}

			memset(t_gcolor_exist, 0, sizeof(int)*color_count);
			for (int g=time_first_group[t];g<time_first_group[t+1];g++) {
				int c = group_color[g];
				t_gcolor_exist[c]++;
			}

			int *tmp_p = curr_time_color_min;
			curr_time_color_min=prev_time_color_min;
			prev_time_color_min=tmp_p;

			int g = ind_time_group[i][t];
			int gc = (g<0?-1:group_color[ind_time_group[i][t]]);

			int t_min=0x7FFFFFFF;
			for (int c_index=0;c_index<i_color_count;c_index++) {

#if COLOR_LOOP_OPT_LV==0
				int c=c_index;
#elif COLOR_LOOP_OPT_LV==1
				if (!is_visited_color[c]) continue;
#elif COLOR_LOOP_OPT_LV==2
				int c=i_color[c_index];
#endif

				int min, prev;
				if (t>0) {
					// not switch
					int cost = prev_time_color_min[c];
					assert(0<=cost);
					assert(cost<0x7FFFFFFF);
					min=cost;
					prev=c;

					// switching IS allowed between absent timesteps
					//		because beta1 cost can force this to happen

					// switch
					for (int d_index=0;d_index<i_color_count;d_index++) {

#if COLOR_LOOP_OPT_LV==0
						int d=d_index;
#elif COLOR_LOOP_OPT_LV==1
						if (!is_visited_color[d]) continue;
#elif COLOR_LOOP_OPT_LV==2
						int d=i_color[d_index];
#endif
						if (c==d) continue;
						// switch cost
						int cost = prev_time_color_min[d] + switch_cost;
						assert(0<=cost);
						assert(cost<0x7FFFFFFF);
						if (min>cost) {
							min=cost;
							prev=d;
						} else if (min==cost && c_index==d_index) {
							min=cost;
							prev=d;
						}
						//printf("i%d t%d c%d d%d cost %d\n", i, t, c, d, cost);
					}

					assert(0<=min);
					assert(min<0x7FFFFFFF);
				} else {
					min=0;
				}

				if (c!=gc) {
					// visit cost
					if (g>=0) {
						min += visit_cost;
					}
					// absence cost
					if (t_gcolor_exist[c]) {
						min += absence_cost;
					}
				}

				curr_time_color_min[c]=min;
				time_color_prev[t][c]=prev;

				if (t_min>min) t_min=min;

			}

		}

		// find min color
		int imin=0x7FFFFFFF;
		int iminc=-1;
		for (int c_index=0;c_index<i_color_count;c_index++) {

#if COLOR_LOOP_OPT_LV==0
			int c=c_index;
#elif COLOR_LOOP_OPT_LV==1
			if (!is_visited_color[c]) continue;
#elif COLOR_LOOP_OPT_LV==2
			int c=i_color[c_index];
#endif
			if (imin>curr_time_color_min[c]) {
				imin=curr_time_color_min[c];
				iminc=c;
			}
		}
		assert(imin<0x7FFFFFFF);
		assert(imin>=0);

		//fprintf(stderr, "i%d cost %d\n", i+1, imin);
		total_cost+= imin;

		// reconstruct coloring
		for (int t=time_count-1;t>0;t--) {
			icolor[t]=iminc;
			iminc = time_color_prev[t][iminc];
		}
		icolor[0]=iminc;

		printf(" ");
		for (int t=0;t<time_count;t++) {
			if (out_color_type==COLOR_1CHAR) {
				printf("%c", color2char(icolor[t]));
			} else if (out_color_type==COLOR_SEP) {
				printf(" %d", icolor[t]);
			} else {
				assert(0);
			}
		}
	}
	printf("\n");

	//fprintf(stderr, "cost %d\n", total_cost);

	for (int t=0;t<time_count;t++) {
		Free(time_color_prev[t]);
	}
}

void process_group_color_limitn(gtm_data *gtm, int *group_color, int group_count, 
	int color_count, int **ind_time_group) {

	const int ind_count = gtm->ind_count;
	const int exist_ind_count = gtm->exist_ind_count;
	const int time_count = gtm->time_count;
	int *group_time = gtm->group_time;

	int time_gcolor_exist[time_count][color_count];
	memset(time_gcolor_exist, 0, sizeof(int)*time_count*color_count);
	for (int g=0;g<group_count;g++) {
		time_gcolor_exist[group_time[g]][group_color[g]]++;
	}

	int ind_gcolor_exist[ind_count][color_count];
	int ind_exist[ind_count];
	int ind_time_exist[ind_count][time_count];
	memset(ind_gcolor_exist, 0, sizeof(int)*ind_count*color_count);
	memset(ind_exist, 0, sizeof(int)*ind_count);
	memset(ind_time_exist, 0, sizeof(int)*ind_count*time_count);
	for (int i=0;i<ind_count;i++) {
		for (int t=0;t<time_count;t++) {
			if (ind_time_group[i][t]<0) continue;
			ind_exist[i]++;
			ind_time_exist[i][t]++;
			ind_gcolor_exist[i][group_color[ind_time_group[i][t]]]++;
		}
	}

	// find all individual colorings

	int oldind[ind_count];
	{
		int newi=0;
		for (int i=0;i<ind_count;i++) {
			if (ind_exist[i]) 
				oldind[i]=newi++;
			else
				oldind[i]=-1;
		} 
	}

	int ind_time_color_min[exist_ind_count][time_count][color_count];
	int ind_time_color_prev_count[exist_ind_count][time_count][color_count];
	int ind_time_color_prev[exist_ind_count][time_count][color_count][color_count];

	int total_cost=0;
	for (int newi=0;newi<exist_ind_count;newi++) {
		int i=oldind[newi];
		assert(i>=0);
		assert(i<ind_count);
		int imin=0x7FFFFFFF;
		for (int t=0;t<time_count;t++) {
			int g = ind_time_group[i][t];
			assertf(g>=-1, "g %d >= -1 violated\n", g);
			assertf(g<group_count, "g %d < group_count %d violated\n", 
				g, group_count);
			int gc = (g<0?-1:group_color[ind_time_group[i][t]]);

			for (int c=0;c<color_count;c++) {
				int min;
				if (t>0) {
					min=0x7FFFFFFF;
					// not switch
					int d = c;
					int cost = ind_time_color_min[newi][t-1][d];
					if (min>=cost) {
						if (min>cost) {
							min=cost;
							ind_time_color_prev_count[newi][t][c]=0;
						}
						
						ind_time_color_prev[newi][t][c][ind_time_color_prev_count[newi][t][c]++]=d;
					}
					//printf("i%d t%d c%d d%d cost %d\n", i, t, c, d, cost);
					// switch
					for (int d=0;d<color_count;d++) {
						if (c==d) continue;
						// switch cost
						int cost = ind_time_color_min[newi][t-1][d] + switch_cost;
						if (min>=cost) {
							if (min>cost) {
								min=cost;
								ind_time_color_prev_count[newi][t][c]=0;
							}
							ind_time_color_prev[newi][t][c][ind_time_color_prev_count[newi][t][c]++]=d;
						}
						//printf("i%d t%d c%d d%d cost %d\n", i, t, c, d, cost);
					}
					assert(min<0x7FFFFFFF);
					assert(min>=0);
				} else {
					min=0;
				}

				if (c!=gc) {
					// visit cost
					if (ind_time_exist[i][t]) {
						min += visit_cost;
					}
					// absence cost
					if (time_gcolor_exist[t][c]) {
						min += absence_cost;
					}
				}

				ind_time_color_min[newi][t][c]=min;
				if (t==time_count-1) {
					if (imin>min) imin=min;
				}
			}
		}
		total_cost += imin;
	}
    if (quiet) {
        printf("%d\n", total_cost);
    } else {
        //fprintf(stderr, "cost %d\n", total_cost);
    }

	// print ind coloring

	Coloring *ind_coloring_first[ind_count];
	Coloring *ind_coloring_last[ind_count];
	memset(ind_coloring_first, 0, sizeof(Coloring*)*ind_count);
	memset(ind_coloring_last, 0, sizeof(Coloring*)*ind_count);

	for (int newi=0;newi<exist_ind_count;newi++) {
		int i=oldind[newi];

		int icolor[time_count];
		int icolors_count[time_count];
		int icolors[time_count][color_count];

		// find min
		int imin=0x7FFFFFFF;
		for (int c=0;c<color_count;c++) {
			if (imin>ind_time_color_min[newi][time_count-1][c]) {
				imin=ind_time_color_min[newi][time_count-1][c];
			}
			assert(imin<0x7FFFFFFF);
			assert(imin>=0);
		}

		// find min colors
		icolors_count[time_count-1]=0;
		for (int c=0;c<color_count;c++) {
			if (imin!=ind_time_color_min[newi][time_count-1][c]) continue;
			icolors[time_count-1][icolors_count[time_count-1]++]=c;
		}

		// enumerate
		int k=time_count-1;
		int icount=0;
		while (k<time_count) {
			while (icolors_count[k]>0) {
				icolor[k] = icolors[k][--icolors_count[k]];
				if (k==0) {
					icount++;
					Coloring *gi = MAlloc(sizeof(Coloring));
					gi->colors = MAlloc(sizeof(int)*time_count);
					for (int t=0;t<time_count;t++) {
						gi->colors[t] = icolor[t];
					}
					gi->next=0;
					if (ind_coloring_last[i]) {
						assert(ind_coloring_first[i]);
						ind_coloring_last[i]->next = gi;
						ind_coloring_last[i] = gi;
					} else {
						assert(!ind_coloring_last[i]);
						ind_coloring_first[i] = gi;
						ind_coloring_last[i] = gi;
					}
				} else {
					// find min colors
					icolors_count[k-1]=0;
					for (int c=0;c<ind_time_color_prev_count[newi][k][icolor[k]];c++) {
						int tmp = ind_time_color_prev[newi][k][icolor[k]][c];
						icolors[k-1][icolors_count[k-1]++]=tmp;
					}
					assert(icolors_count[k-1]>0);
					k--;
				}
			}
			k++;
		}
	}


	// initialize
	Coloring *icolor[ind_count];
	for (int i=0;i<ind_count;i++) {
		if (!ind_exist[i]) continue;
		if (!ind_coloring_first[i]) {
			fprintf(stderr, "Individual %d has no coloring\n", i);
			exit(1);
		}
		icolor[i] = ind_coloring_first[i];
	}

	int icount=0;
	for(;;) {
        if (quiet) break;

		// print one
		for (int g=0;g<group_count;g++) {
			if (out_color_type==COLOR_1CHAR) {
				printf("%c", color2char(group_color[g]));
			} else if (out_color_type==COLOR_SEP) {
				printf(" %d", group_color[g]);
			} else {
				assert(0);
			}
		}
		for (int i=0;i<ind_count;i++) {
			if (!ind_exist[i]) continue;
			printf(" ", i);
			for (int t=0;t<time_count;t++) {
				if (out_color_type==COLOR_1CHAR) {
					printf("%c", color2char(icolor[i]->colors[t]));
				} else if (out_color_type==COLOR_SEP) {
					printf(" %d", icolor[i]->colors[t]);
				} else {
					assert(0);
				}
			}
		}
		printf("\n");
		if (++icount>=ilimit) break;

		// advance to next
		int i;
		for (i=0;i<ind_count;i++) {
			if (!ind_exist[i]) continue;
			if (icolor[i]->next!=0) {
				icolor[i] = icolor[i]->next;
				break;
			} else {
				icolor[i] = ind_coloring_first[i];
			}
		}
		if (i==ind_count) {
			break;
		}
	}

	// clean up
	for (int i=0;i<ind_count;i++) {
		Coloring *gi = ind_coloring_first[i];
		while (gi!=0) {
			Free(gi->colors);
			Free(gi);
			gi=gi->next;
		}
	}
}

int detect_type_from_abc(int limit) {
	if (limit<0) return COLOR_SEP; // use default if too long

	char c1 = getchar();
	if (c1==EOF) {
		return COLOR_SEP;
	}

	int rc=0;
	if ('A'<=c1 && c1<='Z') {
		rc = COLOR_1CHAR;
	} else if (('0'<=c1 && c1<='9') || c1==' ') {
		rc = detect_type_from_abc(limit-1);
	}
	ungetc(c1, stdin);
	return rc;
}

int get_next_char(int *nspace) {
	char c = getchar();
	//printf("%c", c);
	if (c != EOF) {
		if (c!='\n' && c!='\r') {
			int next_char = get_next_char(nspace);
			if (c==' ' && 
				( ('0'<=next_char && next_char<='9') ||
				('A'<=next_char && next_char<='Z') ||
				('a'<=next_char && next_char<='z'))) {

				//printf("'%c%c' ", c, next_char);
				(*nspace)++;
			}
		}
		ungetc(c, stdin);
	}
	return c;
}

int detect_type_by_space_counting() {
	int nspace=0;
	char ch=getchar();
	while (ch==' ') ch=getchar();
	if (ch!=EOF) ungetc(ch, stdin);
	get_next_char(&nspace);
	return nspace;
}

int main(int argc, char*argv[]) {
	char *gtm_fname=0;

	for (int i=1;i<argc;i++) {
		if (argv[i][0]!='-') {
			if (gtm_fname==0) {
				gtm_fname=argv[i];
			} else {
				fprintf(stderr, "ERROR: Only one input file is allowed\n.");
				exit(1);
			}
		} else {
			if (strcmp("-cost", argv[i])==0) {
				const int no_costs=3;
				i++;
				if (argv[i][0]==0) {
					printf("Cost tuple cannot be empty\n");
					exit(1);
				}
				for (int j=0;j<strlen(argv[i]);j++) {
					if (!isdigit(argv[i][j])) {
						printf("Invalid cost tuple %s\n", argv[i]);
						exit(1);
					}
				}
				if (strlen(argv[i])%no_costs!=0) {
					printf("Cost tuple %s length is not a multiple of %d.\n", argv[i], no_costs);
					exit(1);
				}
				int d=strlen(argv[i])/no_costs;
				switch_cost=0; visit_cost=0; absence_cost=0; 
				int j=0;
				for (int k=0;k<d;k++) switch_cost=switch_cost*10+argv[i][j++]-'0';
				for (int k=0;k<d;k++) absence_cost=absence_cost*10+argv[i][j++]-'0';
				for (int k=0;k<d;k++) visit_cost=visit_cost*10+argv[i][j++]-'0';
				//fprintf(stderr, "%d %d %d\n", switch_cost, absence_cost, visit_cost);

			} else if (strcmp("-switch", argv[i])==0) {
				assert(i+1<argc);
				switch_cost=atoi(argv[++i]);
			} else if (strcmp("-diff", argv[i])==0) {
				assert(i+1<argc);
				visit_cost=atoi(argv[++i]);
			} else if (strcmp("-absence", argv[i])==0) {
				assert(i+1<argc);
				absence_cost=atoi(argv[++i]);
			} else if (strcmp("-glimit", argv[i])==0) {
				assert(i+1<argc);
				glimit=atoi(argv[++i]);
				assert(glimit>=0);
			} else if (strcmp("-ilimit", argv[i])==0) {
				assert(i+1<argc);
				ilimit=atoi(argv[++i]);
				assert(ilimit>=0);
			} else if (strcmp("-1char", argv[i])==0) {
				color_type = COLOR_1CHAR;
			} else if (strcmp("-sep", argv[i])==0) {
				color_type = COLOR_SEP;
			} else if (strcmp("-q", argv[i])==0) {
                quiet = 1;
                ilimit = 0;
			} else {
				fprintf(stderr, "Unknown option %s\n", argv[i]);
				print_usage(argv[0]);
			}
		}
	}

	if (gtm_fname==0) {
		print_usage(argv[0]);
		exit(1);
	}

	if (access(gtm_fname, F_OK)!=0) {
		fprintf(stderr, "File %s does not exist\n", gtm_fname);
		exit(1);
	}

	gtm_data gtm;
	if (load_gtmfile(gtm_fname, &gtm)) {
		printf("Error loading gtm file: %s\n", gtm_fname);
		exit(1);
	}

	const int group_count = gtm.group_count;
	const int ind_count = gtm.ind_count;
	const int time_count = gtm.time_count;
	const int *group_time = gtm.group_time;
	const int *group_size = gtm.group_size;

	int *ind_exists = gtm.ind_exists;

	int *ind_time_group[ind_count];
	for (int i=0;i<ind_count;i++) {
		ind_time_group[i] = MAlloc(sizeof(int)*time_count);
		for (int t=0;t<time_count;t++) {
			ind_time_group[i][t]=-1;
		}
	}
	linked_group *p=gtm.group_array[0];
	for (int i=0;p!=0;p=p->next) {
		for (int j=0;j<p->member_count;j++) {
			//printf("i %d t %d g %d\n", p->member[j], p->timestep, i);
			ind_time_group[p->member[j]][p->timestep]=i;
		}
		i++;
	}
	//
	//for (int i=0;i<ind_count;i++) {
	//	for (int t=0;t<time_count;t++) {
	//		if (ind_time_group[i][t]<0) {
	//			printf(" %2c", '.');
	//		} else {
	//			printf(" %2d", ind_time_group[i][t]);
	//		}
	//	}
	//	printf("\n");
	//}
	//


	int group_color[group_count];
	memset(group_color, -1, sizeof(int)*group_count);

	// detect color_type from stdin
	if (color_type==COLOR_DEFAULT) {
		color_type = detect_type_from_abc(100);

		if (color_type==COLOR_DEFAULT) {
			int nspace = detect_type_by_space_counting();
			if (group_count-1 == nspace) {
				color_type = COLOR_SEP;
			} else if (time_count-1 == nspace) {
				color_type = COLOR_1CHAR;
			} else {
				assertf(color_type, "Cannot detect color type.\n"
					"ind_count %d time_count %d group_count %d nspace %d.\n"
					"Use -sep or -1char.", 
					ind_count, time_count, group_count, nspace);
			}
			//fprintf(stderr, "detect %s\n", 
			//	color_type==COLOR_1CHAR?"1char":(color_type==COLOR_SEP?"sep":"unknown"));
		}
	}

	int gcount=0;
	for (;;) {
		// read a line of group colors
		int rc = read_group_color(&gtm, group_color);
		int color_count=0;
		for (int g=0;g<group_count;g++) {
			if (group_color[g]>color_count) color_count=group_color[g];
		}
		color_count++;
		//if (color_count>ind_count) {
		//	fprintf(stderr, "WARNING! color_count %d > ind_count %d\n", 
		//		color_count, ind_count);
		//}

		if (rc==-1) break;
		if (rc==0) continue;

		if (ilimit==1) {
			process_group_color_limit1(&gtm, group_color, group_count, 
				color_count, &ind_time_group[0]);
			break;
		} else {
			process_group_color_limitn(&gtm, group_color, group_count, 
				color_count, &ind_time_group[0]);
			if (++gcount >= glimit) break;
			printf("\n");
		}
	}

	return 0;
}
