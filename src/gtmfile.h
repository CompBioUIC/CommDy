#ifndef _tgifile_h_
#define _tgifile_h_

/***** BEGIN linked_group *****/
typedef struct linked_group_st {
	int group;
	int timestep;
	int *member;
	int member_count;
	struct linked_group_st *next;
} linked_group;

void clear_linked_group(linked_group *first);

linked_group** read_gtmfile(char const *filename, int *group_count);

/***** END linked_group *****/

/***** BEGIN gtmfile *****/
typedef struct gtm_data_st {
	linked_group **group_array;
	int group_count, ind_count, time_count, exist_ind_count;
	int *group_time, *group_size, *ind_exists;
	int *time_first_group, *time_size;
	int max_time_size;
} gtm_data;

int load_gtmfile(char const *filename, gtm_data *gtm);
void finalize_gtm(gtm_data *gtm);
/***** END gtmfile *****/

/***** BEGIN linked_int *****/
typedef struct linked_int_st {
	int data;
	struct linked_int_st *next;
} linked_int;

void clear_linked_int(linked_int *first);

/***** END linked_int *****/

void printGroupColor(int *group_color, int group_count, const int *group_time, int format);
void fprintGroupColor(FILE* f, int *group_color, int group_count, const int *group_time, int format);

#endif
