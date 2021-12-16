#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


//#include "selist.c"
#include "simclist.h"
#include "simclist.c"

#include "uthash.h"

#include "utlist.h"

//#include "tree.c"


int isWord(char *string) {

	for (int i = 0; i < strlen(string); i++) {
		//printf("%c ", string[i]);
		if (!(isalpha(string[i]))) {
			return 0;
		}
	}

	return 1;
}

void strlower(char *string) {
	if (!string) return;
	for (int i = 0; i < strlen(string); i++) {
		string[i] = tolower(string[i]);
	}
}

/*
typedef struct list {
	int ind;
	struct list *next;
} list;
*/

typedef struct set {
	int ind;
	UT_hash_handle hh;
} set;

typedef struct papers {
	char word[70];
	//struct list *head;
	struct set *indSet;
	UT_hash_handle hh;
} papers;

struct papers *p = NULL;

void add_user(char *word) {
	struct papers *s;

	//struct list *tmp = NULL;
	HASH_FIND_STR(p, word, s);
	if (s == NULL) {
		s = malloc(sizeof(struct papers));
		strcpy(s->word, word);
		s->indSet = NULL;
		//list *tmp = malloc(sizeof(struct list));
		HASH_ADD_STR(p, word, s);
	}
	else {
		printf("found non unqiue\n");
	}
}

void delete_inds(struct papers *ex) {
	struct set *p = ex->indSet;
	struct set *current_user, *tmp;

	HASH_ITER(hh, p, current_user, tmp) {
		HASH_DEL(p, current_user);
		free(current_user);
	}
}

void delete_all() {
	struct papers *current_user, *tmp;

	HASH_ITER(hh, p, current_user, tmp) {
		//list_destroy(current_user->l);
		delete_inds(current_user);
		HASH_DEL(p, current_user);
		free(current_user);
	}
}

void print_p() {
	struct papers *s;
	for (s = p; s!= NULL; s=s->hh.next) {
		printf("words %s\n", s->word);
	}
}

//int inList(struct papers *s, int val) {
//	struct list *elt, *t;
//	LL_FOREACH_SAFE(s->head, elt, t) {
//		if (val == elt->ind) return 1;
//	} 
//	return 0;
//
//}

int inHashINT(struct set *p, int index) {
	struct set *s;
	HASH_FIND_INT(p, &index, s);
	if (s == NULL) {
		return 0;
	}
	else {
		return 1;
	}
}
void add_ind(char *string, int ind) {
	struct papers *s;
	HASH_FIND_STR(p, string, s);
	// Check if in the list already
	if ((inHashINT(s->indSet, ind))) {
		return;
	}
	struct set *tmp;
	tmp = malloc(sizeof (struct set));
	tmp->ind = ind;	
	HASH_ADD_INT(s->indSet, ind, tmp);
}
/*
void add_ind(char *string, int ind) {
	struct papers *s;

	//struct list *tmp = NULL;
	HASH_FIND_STR(p, word, s);
	if (s == NULL) {
		s = malloc(sizeof(struct papers));
		strcpy(s->word, word);
		s->indSet = NULL;
		//list *tmp = malloc(sizeof(struct list));
		HASH_ADD_STR(p, word, s);
	}
}
*/

void print_inds(char *string) {
	struct papers *s;
	
	HASH_FIND_STR(p, string, s);
	
	struct set *l, *current_user, *tmp;
	l = s->indSet;
	/*
	for (s = p; s!= NULL; s=s->hh.next) {
		printf("words %s\n", s->word);
	}
	*/
	HASH_ITER(hh, l, current_user, tmp) {
		printf("%d ", current_user->ind);
	}
	printf("\n");

}

int inHash(char *string) {
	struct papers *s;
	HASH_FIND_STR(p, string, s);
	if (s == NULL) {
		return 0;
	}
	else {
		return 1;
	}
}

int main() {
	//list_t mylist;
	//list_init(&mylist);

	///list_attributes_copy(&mylist, list_meter_string, 1);
	//list_attributes_comparator(&mylist, list_comparator_string);


	FILE * fp;
	fp = fopen("wordlist.txt", "r");
	char buf[255];
	int count = 0;
	int index = 0;
	while (fgets(buf, 255, fp)) {
		buf[strlen(buf)-1] = '\0';
		add_user(buf);
		//printf("%s\n", buf);
	}
	//print_p();
	//

	//LL_FOREACH() {

	//}
//	add_ind("matrix", 1);
//	add_ind("matrix", 2);
//	add_ind("matrix", 2);

//	print_inds("matrix");

	//struct tree *bin;
	//int t = list_iterator_start(&mylist);

	//void *data;
	//while (data = list_iterator_next(&mylist)) {
		//((char*)data)[strlen(data)] = '#';
		//printf("%s\n", data);
		
		//printf("LENGTH: %d\n", strlen(data));
		//bin = tree_insert(bin, data, NULL);
	//}

	//tree_print(bin, 0);
	//printf("LEVEL %d\n", bin->level);
	//tree_free(bin);

	FILE *fpl;
	fpl = fopen("arxiv-metadata.txtDGFP", "r");

	char buf2[221072];
	while (fgets(buf2, 221072, fpl)) {
		buf2[strlen(buf2)-1] = '\0';
		//printf("%s\n", buf);
		if (count % 100000 == 0 && index > 0) {
				printf("count %d\n", count);
		}

		// id
		if (count % 5 == 0) {
		}
		// title
		else if (count % 5 == 1) {
			char *string = strtok(buf2, " ");
			strlower(string);
			while (string != NULL) {
				if (isWord(string)) {
					if (inHash(string)) {
						add_ind(string, index);
					}
				}
				string = strtok(NULL, " ");
				strlower(string);
			}	
		}
		// author	
		else if (count % 5 == 2) {
		}
		// abstract
		else if (count % 5 == 3) {
			char *string = strtok(buf2, " ");
			strlower(string);
			while (string != NULL) {
			//	list_append(&mylist, string);	
				if (isWord(string)) {
					if (inHash(string)) {
						add_ind(string, index);
					}
				}
				//printf("z: %s\n", string);
				string = strtok(NULL, " ");
				strlower(string);
			}	
		}
		// +++
		else {
			index++;
		}
		count++;

	}
	
	//list_destroy(&mylist);
	//list_destroy(&mylist);
	
	print_inds("botussinesq");
	delete_all();
	fclose(fp);
	fclose(fpl);
	
}

