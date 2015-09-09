// 
// Branch-and-bound exhaustive search with prioritized search directions
// 
// Memos:
//  - binary-tree heap is still buggy (possibly memory leak), but array heap works fine
//  - catch SIGKILL or SIGTERM and write heap content to log file + resume the run
// 
// 

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include"assert.h"
#include"gtmfile.h"
#include"util.h"
#include"ExhaustiveSearch.h"

int max_color=DEFAULT_MAX_COLOR;
char *time_limit_str="1s";
int time_limit=1;

void print_usage(char *prog_name) {
	printf("usage: %s [<options>] <gtm file>\n", prog_name);
	printf("	-cost : specify cost tuple (i, a, g)\n");
	printf("	-maxcolor : specify the maximum color\n");
	printf("	-timelimit : specify time limit e.g. 1h, 1m, and 1s. Default %s\n", time_limit_str);
	printf("\n");
	exit(0);
}

int main(int argc, char *argv[]) {

	char *gtm_fname=0;
	int exact=0;
	int switch_cost=1, absence_cost=1, diff_cost=1;

	// process arguments
	
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
			} else if (strcmp("-timelimit", argv[i])==0) {
				time_limit_str = argv[++i];
				char *p = time_limit_str + strlen(time_limit_str)-1;
				if (*p == 'h') {
					time_limit = 3600;
				} else if (*p == 'm') {
					time_limit = 60;
				} else if (*p == 's' || ('0'<=*p && *p<='9')) {
					time_limit = 1;
				} else {
					printf("Unknown option time limit %s\n", time_limit_str);
					exit(0);
				}
				time_limit *= atoi(time_limit_str);
			} else if (strcmp("-maxcolor", argv[i])==0) {
				exact=0;
                max_color=atoi(argv[++i]);
			} else if (strcmp("-xmaxcolor", argv[i])==0) {
				exact=1;
                max_color=atoi(argv[++i]);
			} else {
				printf("Unknown option %s\n", argv[i]);
				print_usage(argv[0]);
			}
		}
	}

	// load input files

	assertf(!exact, "Using exact number of colors should not be used when computing lowerbounds.\n");

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
	const int max_time_size= gtm.max_time_size;
	int *time_first_group = gtm.time_first_group;

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

	// debug info
	printf("%d group(s) %d timestep(s)\n", group_count, time_count);
	printf("max color %d	exact %s\n", max_color, exact?"on":"off");
	printf("max time size %d\n", max_time_size);
	printf("cost %d %d %d\n", switch_cost, absence_cost, diff_cost);
	if (time_limit<=0) {
		printf("no time limit\n");
	} else {
		printf("time limit %s (%d sec)\n", time_limit_str, time_limit);
	}

	printf("compute interval lowerbounds\n");

	// init
	int* t1t2_lb[time_count+1];
	int* sum_lbs[time_count+1];
	int* ex_lbs[time_count+1];
	char* lb_kind[time_count+1];
	for (int t=0;t<=time_count;t++) {
		t1t2_lb[t] = malloc((time_count+1) * sizeof(int));
		sum_lbs[t] = malloc((time_count+1) * sizeof(int));
		ex_lbs[t] = malloc((time_count+1) * sizeof(int));
		lb_kind[t] = malloc((time_count+1) * sizeof(char));
	}

	// compute the thing
	compute_interval_lowerbound(&gtm, switch_cost, absence_cost, diff_cost, 
		max_color, time_limit, 2, 
		t1t2_lb, sum_lbs, ex_lbs, lb_kind);

	// print table
	printf("done\n");

	printf("time lowerbound:");
	for (int t=0;t<time_count;t++) {
		printf(" %d", t1t2_lb[t][time_count]);
	}
	printf("\n");

	// clean up after (for what?)
	finalize_gtm(&gtm);
	for (int t=0;t<=time_count;t++) {
		free(t1t2_lb[t]);
		free(sum_lbs[t]);
		free(ex_lbs[t]);
		free(lb_kind[t]);
	}

	return 0;
}

