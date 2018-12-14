#include <stdlib.h>
#include <time.h>
//sscanf (str,"%p",&p)
//p = (void *) stitall ; Unsigned long

#ifndef LIST_H_
#define LIST_H_


struct node {
    void *addr;
    size_t size;
    char* file;
    time_t time;
    struct node *next;
    int type;
    int keyDF;
};

typedef struct node tnode;

typedef struct node *pnode;

typedef struct node * list;

int InsertMallocNode(list *l, void * addr,size_t size);
int InsertMmapNode(list *l, void * addr, char * file);
int InsertSharedNode(list *l,void * addr,int size,int clave);
int PrintListByType(list l, int type);
void DeleteNodeMalloc(list *l,size_t size);
void DeleteNodeMMap(list *l,char * file);
void DeleteNodeShared(list *l,int key);
void Deallocate(list *l,void * addr);

#endif
