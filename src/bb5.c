/*
 * Branch-and-bound exhaustive search with prioritized search directions
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include"assert.h"
#include"gtmfile.h"
#include"util.h"
#include"ExhaustiveSearch.h"

#define DEFAULT_MAX_COLOR 9

int wsize=8;
char *time_limit_str="none";
int max_color=DEFAULT_MAX_COLOR;
int print_info=2; // 0 - nothing, 1 - short, 2 - long

//char *debug_gcolor_str ="121232452521245135161513712761617713178125356195A1A5173717173B7B11157312254";

void print_usage(char *prog_name) {
	printf("usage: %s [<options>] <gtm file>\n", prog_name);
	printf("	-cost : specify cost tuple (i, a, g)\n");
	printf("	-min : specify initial min value\n");
	printf("	-maxcolor : specify the maximum color\n");
	printf("	-xmaxcolor : specify the required maximum color\n");
	printf("	-wsize : specify size of subinterval for tail lowerbounding (0 to disable, default 5)\n");
	printf("	-timelimit : specify time limit e.g. 10h, 10m, and 10s\n");
	printf("	-tlb -tlbfile: tail lowerbounds as a double-quoated string\n");
	printf("	-subtime t1 t2 : limit the search on the subinterval of time\n");
	printf("\n");
	exit(0);
}


int main(int argc, char *argv[]) {

	char *gtm_fname=0;
	char *tlb_str=0;
	char *tlb_fname=0;
	int global_min=0x7FFFFFFF;
	int exact=0;
	int time_limit=0;
	int switch_cost=1, absence_cost=1, diff_cost=1;
	int subt1=-1, subt2=-1;

	for (int i=1;i<argc;i++) {
		if (argv[i][0]!='-') {
			if (gtm_fname==0) {
				gtm_fname=argv[i];
			} else {
				printf("ERROR: Only one input file is allowed\n.");
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
				switch_cost=0; diff_cost=0; absence_cost=0; 
				int j=0;
				for (int k=0;k<d;k++) switch_cost=switch_cost*10+argv[i][j++]-'0';
				for (int k=0;k<d;k++) absence_cost=absence_cost*10+argv[i][j++]-'0';
				for (int k=0;k<d;k++) diff_cost=diff_cost*10+argv[i][j++]-'0';

			} else if (strcmp("-switch", argv[i])==0) {
				switch_cost=atoi(argv[++i]);
			} else if (strcmp("-diff", argv[i])==0) {
				diff_cost=atoi(argv[++i]);
			} else if (strcmp("-absence", argv[i])==0) {
				absence_cost=atoi(argv[++i]);
			} else if (strcmp("-wsize", argv[i])==0) {
				wsize=atoi(argv[++i]);
			} else if (strcmp("-timelimit", argv[i])==0) {
				time_limit_str = argv[++i];
				char *p = time_limit_str + strlen(time_limit_str)-1;
				if (*p == 'd') {
					time_limit = 24*3600;
				} else if (*p == 'h') {
					time_limit = 3600;
				} else if (*p == 'm') {
					time_limit = 60;
				} else if (*p == 's' || ('0'<=*p && *p<='9')) {
					time_limit = 1;
				} else {
					printf("Unknown option time limit %s\n", time_limit_str);
					exit(0);
				}
				//*p = 0;
				time_limit *= atoi(time_limit_str);
			} else if (strcmp("-min", argv[i])==0) {
				global_min=atoi(argv[++i]);
			} else if (strcmp("-maxcolor", argv[i])==0) {
				exact=0;
                max_color=atoi(argv[++i]);
			} else if (strcmp("-xmaxcolor", argv[i])==0) {
				exact=1;
                max_color=atoi(argv[++i]);
			} else if (strcmp("-tlbfile", argv[i])==0) {
				tlb_fname=argv[++i];
			} else if (strcmp("-tlb", argv[i])==0) {
				tlb_str=argv[++i];
			} else if (strcmp("-subtime", argv[i])==0) {
				subt1=atoi(argv[++i]);
				subt2=atoi(argv[++i]);
				assert(subt1<subt2);
			} else {
				printf("Unknown option %s\n", argv[i]);
				print_usage(argv[0]);
			}
		}
	}

	if (gtm_fname==0) {
		print_usage(argv[0]);
		exit(1);
	}

	if (access(gtm_fname, F_OK)!=0) {
		printf("Input file %s does not exist\n", gtm_fname);
		exit(1);
	}

	// load gtm file
	gtm_data gtm;
	if (load_gtmfile(gtm_fname, &gtm)) {
		printf("Error loading gtm file: %s\n", gtm_fname);
		exit(1);
	}

	const int group_count = gtm.group_count;
	const int ind_count = gtm.ind_count;
	const int time_count = gtm.time_count;
	const int max_time_size = gtm.max_time_size;
	const int exist_ind_count = gtm.exist_ind_count;
	int *time_first_group = gtm.time_first_group;
	const int *group_time = gtm.group_time;
	const int *group_size = gtm.group_size;
	linked_group **group_array = gtm.group_array;

	if (subt1==subt2 && subt1==-1) {
		// default
		subt1 = 0;
		subt2 = time_count;
	}

	if (max_color<=0) {
        for (int t=0;t<=time_count;t++) {
            if (time_first_group[t+1]-time_first_group[t]>max_color) {
                max_color = time_first_group[t+1]-time_first_group[t];
            }
        }
	}
	if (max_time_size>max_color) {
		printf("Not enough colors. Need at least %d colors.\n", max_time_size);
		exit(1);
	}

	if (exact && group_count<max_color) {
		printf("Too many colors while exact is on: %d groups, %d group colors", 
			group_count, max_color);
		exit(1);
	}

	/* debug
	linked_group *p;
	for (p=*group_array;p!=0;p=p->next) {
		printf("group %d time %d:", p->group, p->timestep);
		for (int i=0;i<p->member_count;i++) {
			printf(" %d", p->member[i]);
		}
		printf("\n");
	}
	*/

	if (print_info>=2) {
		printf("%d group(s) %d timestep(s)\n", group_count, time_count);
		printf("%d observed individual(s) from %d allocated IDs\n", exist_ind_count, ind_count);
		printf("max color %d	exact %s\n", max_color, exact?"on":"off");
		printf("max time size %d\n", max_time_size);
		printf("cost %d %d %d\n", switch_cost, absence_cost, diff_cost);
		printf("initial min %d\n", global_min);
		if (time_limit<=0) {
			printf("no time limit\n");
		} else {
			printf("time limit %s (%d sec)\n", time_limit_str, time_limit);
		}
	}

	int min_group_color[group_count];

	int *time_lowerbound=0;

    // read/compute lower bounds
	if (tlb_str || tlb_fname) {
		time_lowerbound = CAlloc(time_count+1, sizeof(int));
		int index=0;

		if (tlb_fname) {
			if (print_info>=2) printf("tlbfile %s:", tlb_fname);
			FILE *fp = fopen(tlb_fname, "r");
			if (fp>0) {
				while (index<=time_count) {
					if (fscanf(fp, "%d", &time_lowerbound[index])!=1) break;
					index++;
				}
				fclose(fp);
				if (index==0) {
					fp=NULL;
				} else if (index!=time_count) {
					printf("ERROR: %d numbers are expected in tlb file %s but only %d found\n", 
							time_count, tlb_fname, index);
					exit(1);
				}

			}

			if (fp<=0) { // possibly file doesn't exists
				fprintf(stderr, "tlbfile %s doesn't exist.\n", tlb_fname);
				exit(EXIT_FAILURE);
			}

		} else {
			if (print_info>=2) printf("tlb:");
			while (index<=time_count) {
				if (sscanf(tlb_str, "%d", &time_lowerbound[index])!=1) break;
				index++;

				// advance tlb_str
				while(*tlb_str!=0 && ('0' > *tlb_str || *tlb_str > '9')) tlb_str++;
				while(*tlb_str!=0 && '0' <= *tlb_str && *tlb_str <= '9') tlb_str++;
			}
		}

		while (index<=time_count) {
			time_lowerbound[index++]==0;
		}

		if (print_info>=2) {
			// print tlb
			for (int t=0;t<=time_count;t++) {
				printf(" %d", time_lowerbound[t]);
			}
			printf("\n");
		}
	} else {
		if (wsize>0) {
			if (print_info>=2) printf("tail lowerbounding enabled: window size %d\n", wsize);

			time_lowerbound = CAlloc(time_count+1, sizeof(int));

			int lbs[time_count+1];
			memset(lbs, 0, sizeof(int)*(time_count+1));
			if (print_info>=2) printf("compute tail lowerbounds\n");
			for (int t1=subt2;(t1-=wsize)>=subt1;) {
				int g0=time_first_group[t1], gn=time_first_group[t1+wsize];
				if (print_info>=2) printf("g%d t%d -- g%d t%d ", g0+1, t1+1, gn, t1+wsize);
				fflush(stdout);
				int lb=0x7FFFFFFF;
				int rc = exhaustiveSearch(switch_cost, absence_cost, diff_cost,
						g0, group_array, gn, time_lowerbound, time_count, 
						t1, t1+wsize, 
						group_size, group_time, ind_count, &lb, min_group_color, 
						0, max_color<gn?max_color:gn, time_limit, 0, 1);
				if (print_info>=2) printf("lb %d\n", lb);
				for (int t=0;t<time_count;t++) {
					if (t1-t<0) break;
					lbs[t1-t] += lb;
				}
			}
			memcpy(time_lowerbound, lbs, sizeof(int)*(time_count+1));

			if (print_info>=2) {
				printf("time lowerbound:");
				for (int t=subt1;t<=subt2;t++) {
					printf(" %d", time_lowerbound[t]);
				}
				printf("\n");
			}

			//return 0;
		} else {
			if (print_info>=2) printf("tail lowerbounding disabled\n");
		}
	}

    // run exhaustive search
	if (print_info>=2) printf("start main loop\n");
	fflush(stdout);

	int g1=time_first_group[subt1], g2=time_first_group[subt2];
	global_min = exhaustiveSearch(switch_cost, absence_cost, diff_cost,
			g1, group_array, g2, time_lowerbound, time_count, 
			subt1, subt2, 
			group_size, group_time, ind_count, &global_min, min_group_color, 
			exact, max_color, time_limit, print_info, 1);

	clear_linked_group(*group_array);
	Free(group_array);

	if (time_lowerbound) Free(time_lowerbound);

	return 0;
}
