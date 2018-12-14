/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "list.h"
#include <sys/mman.h>
#include <unistd.h>
#include <sys/shm.h>


pnode CreateNode(void * addr,size_t size,char *t,int keyDF,int type)
{
    pnode p = malloc(sizeof(tnode));

    if (p== NULL)
    {
        return NULL;
    }
    else
    {
        time_t mytime;
        mytime = time(NULL);

        p->addr=addr;
        p->size=size;
        p->time=mytime;
        p->file = t==NULL? NULL : strdup(t); //string
        p->next=NULL;
        p->type=type;
		p->keyDF=keyDF;
    }
    return p;
}
int InsertNode(list **l, void * addr,size_t size,char *t,int keyDF,int type)
{

    pnode p =CreateNode(addr,size,t,keyDF,type);
    if (p==NULL) return -1;
    else
    {
        p->next=**l;

        **l=p;
        ;
        return 1;
    }

}

int InsertMallocNode(list *l, void * addr,size_t size)
{
    return InsertNode(&l,addr,size,NULL,0,0);
}

int InsertSharedNode(list *l,void * addr,int size,int clave)
{

	return InsertNode(&l,addr,size,NULL,clave,2);
}

int InsertMmapNode(list *l, void * addr, char * f)
{

	struct stat sb;
	if (lstat(f,&sb) == -1) {
		perror("stat");
		return -1 ;
	}
		
	FILE *fp;
	fp=fopen(f,"r");
	
	int filedescriptor=fileno(fp);
			

	fclose(fp);
	


 return InsertNode(&l,addr,sb.st_size,f,filedescriptor,1);
}



int PrintListByType(list l, int type)
{

    pnode p=l;
    char* tempstr;
    char buff[20];

    while (p!=NULL)
    {

        if ( p->type==type)
        {
			 printf("%p",p->addr);
            if (type==0)
            {
                tempstr="malloc";
				printf("\tsize: %15lu. %.15s ",p->size,tempstr);
			}
			if (type==1)
			{
				tempstr="mmap";
				printf("  size: %15lu. %.15s %s (fd:%d) ",p->size,tempstr,p->file,p->keyDF);
			}
			if (type==2) 
			{
				tempstr="shared memory";
				printf("  size: %15lu. %.15s (key %d) ",p->size,tempstr,p->keyDF);
			}
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&p->time));
            printf("%s \n",buff);
        }
        p=p->next;

    }
    return 1;
}
void DealocateMemory(pnode p)
{
    int type = p->type;
	
    if (type==0)
    {
        free(p->addr);
    }
	 if (type==1)
	{
		munmap(p->addr,p->size);
		close(p->keyDF);
	}
	if (type==2)
	{
		shmdt(p->addr);
	}
  
}
void DeleteNode(list **l,pnode p)
{
    pnode temp =**l;
    pnode prev;

    if (temp==p)
    {
        **l= temp->next;
        free(temp);
        return;
    }


    while (temp != NULL && temp!=p)
    {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;


    prev->next = temp->next;

    free(temp);
}



void DeleteNodeMMap(list *l,char * file){
	 pnode p=*l;
    while (p!=NULL)
    {
        if ( strcmp(p->file,file)==0)
        {
					
            DealocateMemory(p);
            DeleteNode(&l,p);
            return;
        }
        p=p->next;
    }
    PrintListByType(*l,1);
}
void DeleteNodeShared(list *l,int key){
	 pnode p=*l;
    while (p!=NULL)
    {
        if (( p->keyDF==key) && (p->type==2))
        {
					
            DealocateMemory(p);
            DeleteNode(&l,p);
            return;
        }
        p=p->next;
    }
    PrintListByType(*l,2);
}


void DeleteNodeMalloc(list *l,size_t size)
{
    pnode p=*l;
    while (p!=NULL)
    {
        if ( p->size==size)
        {
            DealocateMemory(p);

            DeleteNode(&l,p);
            return;
        }
        p=p->next;
    }
    PrintListByType(*l,0);
}





void Deallocate(list *l,void * addr) {
	pnode p=*l;
       while (p!=NULL) {
		   
		   if (p->addr==addr) {
			    DealocateMemory(p);
				DeleteNode(&l,p);
			   return;
		   }
		p=p->next;
		}
		PrintListByType(*l,0);
		PrintListByType(*l,1);
		PrintListByType(*l,2);
}
