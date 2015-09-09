#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"assert.h"
#include"util.h"
#include"gtmfile.h"
#include"IntArrayList.h"

// for making IDs start at 0 instead of 1
#define SUBTRACT_ID_BY_ONE

void clear_linked_group(linked_group *first) {
	linked_group *prev;
	while (first!=0) {
		prev=first;
		first=first->next;
		if (prev->member!=0) Free(prev->member);
		Free(prev);
	}
}

void clear_linked_int(linked_int *first) {
	linked_int *prev;
	while (first!=0) {
		prev=first;
		first=first->next;
		Free(prev);
	}
}

int get_int(FILE *fp, int *eol) {
	char ch;
	int rc=0;
	ch=getc(fp);
	if (ch==EOF) return EOF;
	do {
		assertf('0'<=ch && ch<='9', "Invalid character '%c' (%d)", ch, ch);
		rc = rc*10 + ch-'0';
		ch=getc(fp);
	} while (ch!=EOF && ch!='\n' && ch!='\r' && ch!='#' && ch!=' ');
	if (ch=='#') while ((ch=getc(fp))!=EOF && ch!='\n' && ch!='\r');
	if (ch=='\n' || ch=='\r') {
		*eol = 1;
		ch=getc(fp);
		if (ch!='\n' && ch!='\r' && ch!=EOF) {
			ungetc(ch, fp);
		}
	} else {
		*eol = 0;
	}
	if ((ch=getc(fp))!=EOF) {
		int ok=ungetc(ch, fp);
		assert(ok);
	}
	return rc;
}

linked_group** read_gtmfile(char const *filename, int *group_count) {

	int debug=0;

	if (strcmp(filename+strlen(filename)-4, ".gtm")!=0) {
		fprintf(stderr, "File %s doesn't have gtm extension\n", filename);
		exit(1);
	}

	FILE *fp=0;
	if (strcmp(filename, "-")==0) {
		fp = stdin;
	} else {
		if ((fp = fopen(filename, "r")) == 0) {
			fprintf(stderr, "Cannot open input file: %s\n", filename);
			return 0;
		}
	}

	IntArrayList memberList;
	initializeIAL(&memberList);

	linked_group *group_first=0, *group_last=0;
	int const buflen=2000;
	char buf[buflen], tmp[buflen];
	int line_count=0;
	for (;;) {

		int eol;
		int group = get_int(fp, &eol);
		if (group==EOF) break;
		line_count++;
		assert(!eol);

#ifdef SUBTRACT_ID_BY_ONE
		group--;
#endif

		int timestep = get_int(fp, &eol);
#ifdef SUBTRACT_ID_BY_ONE
		timestep--;
#endif

		// debug
		if (debug==1) printf("%s -> g %d t %d\n", buf, group, timestep);

		// read members
		clearIAL(&memberList);
		while (!eol) {
			int individual = get_int(fp, &eol);
#ifdef SUBTRACT_ID_BY_ONE
			--individual;
#endif
			addIAL(&memberList, individual);
		}

		// store in linked list
		if (group_last==0) {
			group_last=MAlloc(sizeof(linked_group));
			group_first=group_last;
		} else {
			group_last->next=MAlloc(sizeof(linked_group));
			group_last=group_last->next;
		}
		group_last->group=group;
		group_last->timestep=timestep;
		//group_last->member_count=count;
		//group_last->member=MAlloc(sizeof(int)*count);
		group_last->member_count=memberList.count;
		group_last->member=MAlloc(sizeof(int) * memberList.count);
		group_last->next=0;

		memcpy(group_last->member, memberList.p, sizeof(int)*memberList.count);
		//// bubble sort the members.
		//{
		//	int *member = group_last->member;
		//	int tmp;
		//	for (int i=memberList.count;i>0;i--) {
		//		for (int j=0;j+1<i;j++) {
		//			if (member[j]>member[j+1]) {
		//				tmp=member[j];
		//				member[j]=member[j+1];
		//				member[j+1]=tmp;
		//			}
		//		}
		//	}
		//}

//		while (first!=0) {
//			last=first;
//			first=first->next;
//#ifdef SUBTRACT_ID_BY_ONE
//			group_last->member[count++]=last->data-1;
//#else
//			group_last->member[count++]=last->data;
//#endif
//			Free(last);
//		}

		if (debug==2) {
			printf("%s -> group %d time %d: ", buf, group, timestep);
			printIAL(&memberList);
			printf("\n");
			//int i;
			//for (i=0;i<group_last->member_count;i++) {
			//	printf(" %d", group_last->member[i]);
			//}
			//printf("\n");
		}

	}
	finalizeIAL(&memberList);
	fclose(fp);

	// convert linked-list to array
	int count=0;
	linked_group *p=0;
	for (p=group_first;p!=0;p=p->next) {
		count++;
	}
	*group_count=count;
	linked_group **group_array = MAlloc(sizeof(linked_group*) * (count+1));

	count=0;
	for (p=group_first;p!=0;p=p->next) {
		group_array[count++] = p;
	}
	group_array[count] = 0;

	return group_array;
}

int load_gtmfile(const char *filename, gtm_data *gtm) {

	int group_count, ind_count=-1, time_count=-1;
	linked_group **group_array=read_gtmfile(filename, &group_count);
	if (!group_array) {
		printf("Error reading input file\n");
		return 1;
	}

	int *group_time = CAlloc(group_count, sizeof(int));
	int *group_size = CAlloc(group_count, sizeof(int));
	{
		linked_group *p=group_array[0];
		for (int i=0;p!=0;p=p->next) {
			//printf("g %d t%d\n", p->group, p->timestep);
			group_time[i] = p->timestep;
			if (p->timestep>time_count) time_count=p->timestep;
			group_size[i] = p->member_count;
			for (int j=0;j<p->member_count;j++) {
				if (p->member[j]>ind_count) ind_count=p->member[j];
			}
			i++;
		}
	}
	ind_count++;
	time_count++;
	//printf("%d time\n", time_count);

	int *ind_exists = CAlloc(ind_count, sizeof(int));
	memset(ind_exists, 0, ind_count*sizeof(int));
	for (linked_group *p=group_array[0];p!=0;p=p->next) {
		for (int j=0;j<p->member_count;j++) {
			ind_exists[p->member[j]]++;
		}
	}
	int exist_ind_count=0;
	for (int i=0;i<ind_count;i++) {
		if (ind_exists[i]) exist_ind_count++;
	}

	int *time_first_group = CAlloc(time_count+1, sizeof(int));
	for (int t=1;t<time_count;t++) time_first_group[t]=-1;
	time_first_group[0]=0;
	for (int g=1;g<group_count;g++) {
		assert(group_array[g]->timestep>=group_array[g-1]->timestep);
		if (group_array[g]->timestep!=group_array[g-1]->timestep) {
			time_first_group[group_array[g]->timestep] = g;
		}
	}
	time_first_group[time_count]=group_count;
	for (int t=time_count;--t>=0;) {
		if (time_first_group[t]<0) {
			time_first_group[t]=time_first_group[t+1];
		}
	}

	int *time_size = CAlloc(time_count, sizeof(int));
	memset(time_size, 0, sizeof(int)*time_count);
	for (int g=0;g<group_count;g++) {
		time_size[group_array[g]->timestep]++;
	}

	int max_time_size=0;
	for (int t=0;t<time_count;t++) {
		if (time_size[t]>max_time_size) {
			max_time_size = time_size[t];
		}
	}

	// populate gtm_data
	gtm->group_array = group_array;
	gtm->group_count = group_count;
	gtm->ind_count = ind_count;
	gtm->time_count = time_count;
	gtm->exist_ind_count = exist_ind_count;
	gtm->group_time = group_time;
	gtm->group_size = group_size;
	gtm->ind_exists = ind_exists;
	gtm->time_first_group = time_first_group;
	gtm->time_size = time_size;
	gtm->max_time_size = max_time_size;

	return 0;
}

void finalize_gtm(gtm_data *gtm) {
	clear_linked_group(*gtm->group_array);
	Free(gtm->group_time);
	Free(gtm->group_size);
	Free(gtm->ind_exists);
	Free(gtm->time_first_group);
	Free(gtm->time_size);
}

void fprintGroupColor(FILE* f, int *group_color, int group_count, const int *group_time, int format) {
	//fprintf(f, "[");
	if (format!=1) fprintf(f, "[ ");
	for (int i=0;i<group_count;i++) {
		assert(group_color[i]>=0);

		//// format: 1char
		//if (i>0 && group_time[i-1]!=group_time[i]) {
		//	//fprintf(f, " |");
		//	fprintf(f, " ");
		//}
		////fprintf(f, " %d", group_color[i]);

		////if (0 <= group_color[i] && group_color[i] <= 9) {
		//	fprintf(f, "%d", group_color[i]);
		//} else if (10 <= group_color[i] && group_color[i] <= 'Z'-'A'+11) {
		//	fprintf(f, "%c", 'A' + group_color[i] - 10);
		//} else {
		//	fprintf(f, "Too many colors: 0-9, A-Z cannot represent it\n");
		//	exit(1);
		//}
		
		// format: space separated
		if (i>0) fprintf(f, " ");
		fprintf(f, "%d", group_color[i]);
	}
	if (format!=1) fprintf(f, " ]");
	fprintf(f, "\n");
}

void printGroupColor(int *group_color, int group_count, const int *group_time, int format) {
    fprintGroupColor(stdout, group_color, group_count, group_time, format);
}
