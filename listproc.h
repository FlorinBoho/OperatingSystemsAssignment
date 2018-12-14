#include <stdlib.h>
#include <time.h>
#ifndef LIST2_H_
#define LIST2_H_
#define MAXSIZE 256

struct temp {
    pid_t pid;
    int prio;
    char * name;
    time_t time;
    char *  signal;
    int exitcode;
    char *  sigName;
};

typedef struct temp item;


struct field {
		item data[MAXSIZE]; 
		int lastPos;
};

typedef struct field Prolist;	






void CreateEmptyList(Prolist *l);
int Insert(Prolist *l, item aux);
void PrintList(Prolist * l);
void PrintListPid(Prolist * l, pid_t pid);
void DeleteAll(Prolist *l);

#endif

