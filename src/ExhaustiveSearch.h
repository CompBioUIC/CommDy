#ifndef _ExhaustiveSearch_h_
#define _ExhaustiveSearch_h_

int exhaustiveSearch(int switch_cost, int absence_cost, int diff_cost, 
	int g0, linked_group **group_array, int group_count, 
	const int *time_lowerbound, int time_count, 
	int subt1, int subt2, 
	const int *group_size, const int *group_time, int ind_count, 
	int *global_min, int *min_group_color, 
	int exact, int max_color, int time_limit, int print_info, int print_strictly_less_than_min);

void compute_interval_lowerbound(gtm_data *gtm, 
		int switch_cost, int absence_cost, int diff_cost, int max_color, int time_limit, int print_info, 
		int** t1t2_lb, int** sum_lbs, int** ex_lbs, char** lb_kind);

void compute_interval_lowerbound2(gtm_data *gtm, 
		int switch_cost, int absence_cost, int diff_cost, int max_color, int time_limit, int print_info, 
		int** t1t2_lb, int** sum_lbs, int** ex_lbs, char** lb_kind);

#endif
