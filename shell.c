/*


   Shell Operating Systems - Assignment 1

Autors: 	Bohotineanu Florin		f.bohotineanu	ID = <removed>
		Ishwara Coello Escobar		ishwara.coello	ID = <removed>
		Federico Maresca		federico.maresca	ID = <removed>



*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <limits.h> 
#include "list.h"
#include "listproc.h"
#include <sys/mman.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#define MAXNOMBRE 1024
#define MAXSEARCHLIST 128


#define MAX 512
#define LEERCOMPLETO ((ssize_t)-1)
int recursiveFlag=0;
list l=NULL;
Prolist ProL;



char *searchlist[MAXSEARCHLIST]={NULL};
char test='x';

int TrocearCadena(char * cadena, char * trozos[])
{ int i=1;
	if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
		return 0;
	while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
		i++;
	return i;
}

char TipoFichero (mode_t m)
{
	switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
		case S_IFSOCK: return 's'; /*socket */
		case S_IFLNK:  return 'l';  /*symbolic link*/
		case S_IFREG:  return '-'; /* fichero normal*/
		case S_IFBLK:  return 'b'; /*block device*/
		case S_IFDIR:  return 'd'; /*directorio */
		case S_IFCHR:  return 'c'; /*char device*/
		case S_IFIFO:  return 'p'; /*pipe*/
		default: return '?'; /*desconocido, no deberia aparecer*/
	}
}

char * ConvierteModo2 (mode_t m)
{
	static char permisos[12];
	strcpy (permisos,"---------- ");
	permisos[0]=TipoFichero(m);
	if (m&S_IRUSR) permisos[1]='r';
	if (m&S_IWUSR) permisos[2]='w';
	if (m&S_IXUSR) permisos[3]='x';
	if (m&S_IRGRP) permisos[4]='r';
	if (m&S_IWGRP) permisos[5]='w';
	if (m&S_IXGRP) permisos[6]='x';
	if (m&S_IROTH) permisos[7]='r';
	if (m&S_IWOTH) permisos[8]='w';
	if (m&S_IXOTH) permisos[9]='x';
	if (m&S_ISUID) permisos[3]='s';
	if (m&S_ISGID) permisos[6]='s';
	if (m&S_ISVTX) permisos[9]='t';
	return (permisos);
}

void Cmd_Help()
{
	printf("\n");
	printf("%15s %s\n","autores [-l|-n]", "Prints the names and logins of the program authors.");
	printf("%15s %s\n","","autores -l prints only the logins and autores -n prints the names");
	printf("%15s %s\n","pid [-p]","Prints the pid of the process executing the shell");
	printf("%15s %s\n","","pid -p prints the pid of its parent process.");
	printf("%15s %s\n","info","Gives info on the files and or directories supplied as its arguments");
	printf("%15s %s\n","recursive [ON|OFF]","this flag affects how the following command will list directories.");
	printf("%15s %s\n","","If the flag is ON directories will be listed recursively.");
	printf("%15s %s\n","list [-l] name1","lists the directories and/or files supplied to it as command line arguments");
	printf("%15s %s\n","eliminate [-f] ","deletes a file or a directory");
	printf("%15s %s\n","fin","Ends the shell");
	printf("%15s %s\n","end","Ends the shell");
	printf("%15s %s\n","exit","Ends the shell");
}

void printLogins()
{
	printf("f.bohotineanu\n");
	printf("ishwara.coello\n");
	printf("federico.maresca\n");
}

void printNames()
{
	printf("Florin Bohotineanu\n");
	printf("Ishwara Coello Escobar\n");	
	printf("Federico Maresca\n");
}

void Cmd_Pid(char *word[])
{
	if (word[0]==NULL)
		printf("print pid of the process executing the shell %d\n",getpid());
	else if (strcmp(word[0],"-p")==0) printf("print pid of the parent process ot the one executing the shell %d\n",getppid());
	else  printf("Wrong command! You didn't pick up anything from the help list!\n");	
}

void Cmd_Autores(char*word[])
{
	if (word[0]==NULL) {
		printLogins();
		printNames();
	}	
	else if (strcmp(word[0],"-l")==0) printLogins();
	else if (strcmp(word[0],"-n")==0) printNames();
	else  printf("Wrong command! You didn't pick up anything from the help list!\n");
}


char *getDate (time_t t)
{
	char *date=ctime(&t);
	date[16]='\0';
	return date+4;
}
char *getUserName(char *word)
{
	struct stat sb;
	struct passwd *pwd;
	lstat(word,&sb);
	pwd=getpwuid(sb.st_uid);
	if(pwd==NULL)
		return "User not found";
	return pwd->pw_name;
}

char *getGroupName(char *word)
{
	struct stat sb;
	struct group *grp;
	lstat(word,&sb);
	grp=getgrgid(sb.st_gid);
	if(grp==NULL)
		return "Group not found";
	return grp->gr_name;
}

void printLinkRealPath(char *word)
{
	struct stat sb;
	char *actualpath;
	lstat(word,&sb);
	if(TipoFichero(sb.st_mode)=='l')
	{
		actualpath = malloc(sb.st_size + 1);
		readlink(word, actualpath, sb.st_size + 1);		
		printf(" -> %s",actualpath);
		free(actualpath);
	}
}	
void printDetails(char word[])
{
	struct stat sb;
	if (lstat(word,&sb) == -1) {
		perror("stat");
		return;
		//exit(EXIT_FAILURE);
	}
	printf("%8ld ",(long)sb.st_ino);
	printf("%s ",ConvierteModo2(sb.st_mode));
	printf("%ld ",sb.st_nlink);
	printf("%s ",getUserName(word));
	printf("%s ",getGroupName(word));
	printf("%8ld ",(long)sb.st_size);
	printf("%s ",getDate(sb.st_mtim.tv_sec));
	printf("%s",word);
	printLinkRealPath(word);
	printf("\n");
} 

void printLessDetails(char word[])
{
	struct stat sb;
	if (lstat(word,&sb) == -1) {
		perror("stat");
		return;
		//exit(EXIT_FAILURE);
	}
	printf("%8ld ",(long)sb.st_size);
	printf("%s",word);
	printf("\n");
}

void Cmd_Info(char *word[], int counter)
{
	for(int i=0;i<counter;i++)
		printDetails(word[i]);
}

int IsADirectory (char *file)
{
	struct stat s;
	if (lstat(file,&s)==-1) return 0;

	return S_ISDIR (s.st_mode);
}


void PrintFile(char word[], int longlist) {

	if (longlist!=0) 
		printDetails(word);
	else printLessDetails(word);
}

void PrintDirectoryContent(char word[],int longlist) 
{

	struct dirent *dp;
	DIR* dir;
	char aux[PATH_MAX];

	dir=opendir(word); 
	while ((dp=readdir(dir)) != NULL) {
		if ( !(strcmp(dp->d_name, ".")==0) && !(strcmp(dp->d_name, "..")==0) ) {
			sprintf(aux, "%s/%s", word,  dp->d_name);
			// if recursive and its a directory, call PrintDirectoryContent(directory name)
			if (recursiveFlag && IsADirectory(aux)) {
				PrintFile(aux, longlist);
				PrintDirectoryContent(aux,longlist);	
			}			
			else    
				PrintFile(aux, longlist );
		}
	}
	closedir(dir);
}      

void PrintCurrentWorkingDirectory(int longlist) 
{

	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		PrintDirectoryContent(cwd,longlist);
	}  
	else {
		perror("getcwd() error");
	}  

}      


void Cmd_List(char *word[])
{
	int longlist = 0;
	if ( (word[0]!=NULL) && (!strcmp(word[0],"-l")) ) {
		longlist=1;
		++word;
	}
	if (word[0]==NULL) {
		PrintCurrentWorkingDirectory(longlist);
	}  
	else  
	{
		while (word[0]!=NULL) 
		{
			if (IsADirectory(word[0])) 
				PrintDirectoryContent(word[0],longlist);
			else PrintFile(word[0],longlist);
			++word;    
		}
	}
}

void Cmd_Recursive(char *word[])
{
	if(word[0]==NULL)
	{	
		recursiveFlag ? printf("Recursive: ON\n") : printf("Recursive: OFF");
		return;
	}
	if(!strcmp(word[0],"OFF"))
		recursiveFlag=0;
	else if (!strcmp(word[0],"ON"))
		recursiveFlag=1;
}	

int isDirectory (char *word)
{
	struct stat sb;
	if (lstat(word,&sb)==-1) 
		return 0;  
	return S_ISDIR (sb.st_mode);
}

int remove_directory(char *path)
{
	char aux[PATH_MAX];
	DIR  *directory;
	struct dirent *d;
	int flag;
	int p;

	if( (directory=opendir(path))==NULL)
		return -1;
	while((d=readdir(directory))!=NULL)
	{
		if( (!strcmp(d->d_name,".")) || (!strcmp(d->d_name,"..")) )
			continue;
		sprintf(aux,"%s/%s",path,d->d_name);
		flag=isDirectory(aux);
		if(flag)
			p=remove_directory(aux);
		else
			p=unlink(aux);
		if(p==-1)
		{
			perror(aux);
			break;
		}
	}
	closedir(directory);
	return rmdir(path);
}	


void Cmd_Eliminate(char *word[])
{
	int flag=0;
	if( (word[0]!=NULL)&&(!strcmp(word[0],"-f")) )
	{
		flag=1;
		word++;
	}
	if(word[0]==NULL)
	{
		printf("Nothing to eliminate\n");
		return;
	}
	if(!flag)
	{
		if(remove(word[0])==-1)
			printf("Impossible to eliminate!\n");
	}
	else if (remove_directory(word[0])==-1)
		printf("Impossible to eliminate!\n");

}


void DeallocateMalloc(char* word)
{
	if (word==NULL)
		PrintListByType(l,0);
	else
		DeleteNodeMalloc(&l,atoi(word));
}


void Cmd_Malloc(char *word[])
{
	if (word[0]==NULL)	{
		PrintListByType(l,0);
		return;
	}
	if (strcmp(word[0],"-deallocate")==0){
		word++;
		DeallocateMalloc(word[0]);
	}
	else
	{
		int tsize=atoi(word[0]);
		if (tsize==0)
			return;
		else
		{
			void * p = malloc(tsize);
			if (p==NULL)
				printf("no enough memory");
			else
			{
				printf("allocated %d  at %p \n",tsize,p);
				InsertMallocNode(&l,p,tsize);
			}
		}
	}
}

void DeallocateMmap(char *word){
	if (word==NULL)
		PrintListByType(l,1);
	else
		DeleteNodeMMap(&l,word);
}


void * MmapFichero (char * fichero, int protection)
{
	int df, map=MAP_PRIVATE,modo=O_RDONLY;
	struct stat s;
	void *p;
	if (protection&PROT_WRITE)  modo=O_RDWR;
	if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
		return NULL;
	if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
		return NULL;
	InsertMmapNode(&l,p,fichero);

	return p;
}
void Cmd_Mmap (char *arg[])
{
	char *perm;
	void *p;
	int protection=0;
	if (arg[0]==NULL)
	{
		PrintListByType(l,1);
		return;
	}
	else if (strcmp(arg[0],"-deallocate")==0)
	{
		arg++;
		DeallocateMmap(arg[0]);
	}
	else
	{
		if ((perm=arg[1])!=NULL && strlen(perm)<4)
		{
			if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
			if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
			if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
		}
		if ((p=MmapFichero(arg[0],protection))==NULL)
			perror ("Impossible to map the file");
		else 
		{
			printf ("file %s mapped in %p\n", arg[0], p);

		}
	}
}




void * ObtenerMemoriaShmget (key_t clave, off_t tam)
{
	void * p;
	int aux,id,flags=0777;
	struct shmid_ds s;

	if (tam) /*if size isnt 0 you create it in exclusice  mode */
		flags=flags | IPC_CREAT | IPC_EXCL;
	/*if size is 0 tries to access a already existing one*/
	if (clave==IPC_PRIVATE)
		/*not useful*/
	{errno=EINVAL; return NULL;}if ((id=shmget(clave, tam, flags))==-1)
	return (NULL);
	if ((p=shmat(id,NULL,0))==(void*) -1){
		aux=errno;
		/*has been created but cannot be mapped*/
		if (tam)
			/*deletos */
			shmctl(id,IPC_RMID,NULL);
		errno=aux;
		return (NULL);
	}
	shmctl (id,IPC_STAT,&s);
	InsertSharedNode(&l,p,s.shm_segsz,clave);
	return (p);
}
void Cmd_Sharednew (char *arg[])
{
	key_t k;
	off_t tam=0;
	void *p;

	if (arg[0]==NULL || arg[1]==NULL)
	{PrintListByType(l,2); return;}
	k=(key_t) atoi(arg[0]);

	if (arg[1]!=NULL)
		tam=(off_t) atoll(arg[1]);

	if ((p=ObtenerMemoriaShmget(k,tam))==NULL)
		perror ("shmget: imposible to obtain shared memory");
	else
		printf ("Shmget memory of key %d assigned in %p\n",k,p);
}

void DeallocateShared (char * words[])
{
	DeleteNodeShared(&l,atoi(words[0]));
}

void Cmd_Shared (char *words[]){
	void * p;
	if (words[0]==NULL) {
		PrintListByType(l,2);
		return;
	}
	if (strcmp(words[0],"-deallocate")==0){
		DeallocateShared(words+1);
	}
	else 
		if ((p=ObtenerMemoriaShmget(atoi(words[0]),0))==NULL)
			perror ("shmget: imposible to obtain shared memory");
		else
			printf ("Shmget memory of key %d assigned in %p\n",atoi(words[0]),p);

}
void Cmd_rmkey (char *args[])
{
	key_t clave;
	int id;
	char *key=args[0];
	if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE)
	{
		printf ("   rmkey  invalid_class\n");
		return;
	}
	if ((id=shmget(clave,0,0666))==-1)
	{
		perror ("shmget: imposible to obtain shared memory");
		return;
	}
	if (shmctl(id,IPC_RMID,NULL)==-1)
		perror ("shmctl: imposible to erase shared memory\n");
}

void Cmd_Allocation () {
	PrintListByType(l,0);
	PrintListByType(l,1);
	PrintListByType(l,2);
}
void Cmd_Deallocate(char * words[]) 
{
	if (words[0]==NULL) {
		printf("Error Deallocate, invalid argument \n");
		return;
	}
	void * addr;
	sscanf(words[0],"%p",&addr);
	Deallocate(&l,addr); 
}
void Cmd_Mem() 
{
	int one = 12;
	list two;
	char three='a';
	printf("Three Global Variables: %p  %p  %p \n",&l,&test,&recursiveFlag);
	printf("Three Local Variables: %p  %p  %p \n",&one,&two,&three);
	printf("Three function Variables: %p  %p  %p \n",&Cmd_Mem,&Cmd_Help,&Cmd_Allocation);

}

void Cmd_memdump(char * words[]) 
{
	int i,size=25,t=0,min=25;
	char *c;

	if (words[1] != NULL)
		size = atoi(words[1]);
	c= (unsigned char * ) strtoull(words[0],NULL,16);
	while (t < size) {
		for (i=t; i<min+t; i++) 
			if (isprint(c[i]))
				printf("%4c", c[i]);
			else
				printf("    ");
		printf("\n");

		for (i=t; i<min+t; i++) 
			printf("  %02x", c[i]);

		printf("\n");
		t=t+25;
		if (size-t<24)
			min = size-t;
	}
	printf("\n");
}

void recursiva (int n)
{
	char automatico[2048];
	static char estatico[2048];
	printf ("parameter n:%d at 					%p\n",n,&n);
	printf ("static array at:			%p \n",estatico);
	printf ("automatic array at 	%p\n\n",automatico);

	n--;
	if (n>0)
		recursiva(n);
}
void Cmd_recursivefunction (char *words[])
{
	int n;
	if(words[0]==NULL) return;
	n=atoi(words[0]);
	recursiva(n);
}

ssize_t LeerFichero (char *fich, void *p, ssize_t n)  /*n=-1 indica que se lea todo*/
{
	ssize_t  nleidos,tam=n;
	int df, aux;
	struct stat s;
	if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
		return ((ssize_t)-1);
	if (n==LEERCOMPLETO)
		tam=(ssize_t) s.st_size;
	if ((nleidos=read(df,p, tam))==-1){
		aux=errno;
		close(df);
		errno=aux;
		return ((ssize_t)-1);
	}
	close (df);
	return (nleidos);
}
void Cmd_Read(char *words[])  //fich addr cont
{

	if(words[0]!=NULL && words[1]!=NULL ) 
	{
		int count;
		if ( words[2]==NULL ) 	
			count = -1;
		else 
			count = atoi(words[2]);

		void *p;
		sscanf(words[1],"%p",&p);
		LeerFichero (words[0], p,count); 
	}
	else
	{
		printf("you missed some parameters \n");
	}
}
void Cmd_Write(char *words[]) //fich addr cont -o
{
	if(words[0]==NULL || words[1]==NULL || words[2]==NULL) return;
	int exists=0;
	if (access(words[0],F_OK) != -1) exists=1; //does the file already exist? 

	char filename[256];
	strcpy(filename,words[0]);
	words++;	//read fich, and now it's addr cont -o;


	void *p;
	sscanf(words[0],"%p",&p);
	words++;	//read the addr, now it's cont and -o;

	int n=atoi(words[0]);
	words++;	//read the number of bytes, now it's only -o 

	int flag=0;
	if(words[0]!=NULL)
		if(!strcmp(words[0],"-o"))
			flag=1;
	FILE *out;
	out=fopen(filename,"w");
	int fd=fileno(out);

	if (exists && flag==0)  
	{	
		printf("Can't write in an already existing file!\n");
		return;	}
	int ka=write(fd,p,n);
	printf("n=%d\n",n);
	printf("p=%p\n",p);
	printf("write(fd,p,n)=%d\n",ka);
	if(!ka)
		printf("error writing\n");
	else
		printf("succes writing \n");


}

void Cmd_SetPriority(char *words[]) //pid value
{
	if (words[0]!=NULL && words[1]!=NULL) {
		setpriority(PRIO_PROCESS,atoi(words[0]),atoi(words[1]));

	}
	else if (words[1]==NULL) {
		printf(" The priority of the procces with pid : %s is %d . \n",words[0],getpriority(PRIO_PROCESS,atoi(words[0])));
	}
	else 
	{
		printf("You didnt put any argument \n");	
	}


}
void Cmd_Fork() {
	pid_t pid;

	if ((pid=fork())==0) 
		printf("Executing process %d \n",getpid());
	else
		waitpid(pid,NULL,0);	

}

void SearchListShow()
{
	int i;
	for (i=0; searchlist[i]!=NULL; i++)
		printf ("%s\n",searchlist[i]);

}

int SearchListAddDir(char * dir)
{
	int i=0;
	char * p;

	while (i<MAXSEARCHLIST-2 && searchlist[i]!=NULL)
		i++;
	if (i==MAXSEARCHLIST-2)
	{errno=ENOSPC; return -1;} /*no cabe*/
	if ((p=strdup(dir))==NULL)
		return -1;
	searchlist[i]=p;
	searchlist[i+1]=NULL;
	return 0;
}

void SearchListNew()
{
	int i;
	for (i=0; searchlist[i]!=NULL; i++) {
		free(searchlist[i]);
		searchlist[i]=NULL;
	}
}

void SearchListAddPath()
{
	char *aux;
	char *p;
	if ((p=getenv("PATH"))==NULL){
		printf ("Imposible obtener PATH del sistema\n");
		return;
	}
	aux=strdup(p);
	if ((p=strtok (aux,":"))!=NULL && SearchListAddDir(p)==-1)
		printf ("Imposible anadir %s: %s\n", p, strerror(errno));
	while ((p=strtok(NULL,":"))!=NULL)
		if (SearchListAddDir(p)==-1){
			printf ("Imposible anadir %s: %s\n", p, strerror(errno));
			break;
		}
	free(aux);
}

char * SearchExecutable(char * ejec)
{
	static char aux[MAXNOMBRE];
	int i;
	struct stat s;

	if (ejec==NULL)
		return NULL;
	if (ejec[0]=='/' || !strncmp (ejec,"./",2) || !strncmp (ejec,"../",2)){
		if (stat(ejec,&s)!=-1)
			return ejec;
		else
			return NULL;
	}

	for (i=0;searchlist[i]!=NULL; i++){
		sprintf(aux,"%s/%s",searchlist[i],ejec);
		if (stat(aux,&s)!=-1)
			return aux;
	}
	return NULL;
}


void Cmd_SearchList(char *words[])
{
	if (words[0]==NULL) {
		SearchListShow();
		return;
	}
	if (words[0][0]=='+'){
		memmove(words[0], words[0]+1, strlen(words[0]));
		if	(isDirectory (words[0])) 
			SearchListAddDir(words[0]);
		else printf("Not a directory\n");		
		return;	
	}
	if (!strcmp(words[0],"-path")){
		SearchListAddPath();
		return;
	}
	printf("%s\n",SearchExecutable(words[0]));			
} 

void CheckPriorityChange (char *args[])
{
	int i,pri;

	for (i=0; args[i]!=NULL; i++);

	if (i==0)
		return;
	if (args[i-1][0]=='@'){
		pri=atoi(args[i-1]+1);
		if (setpriority (PRIO_PROCESS, getpid(), pri)==-1)
			perror ("Cannot change priority");
		args[i-1]=NULL;
	}
}



void Cmd_Exec(char *words[]) 
{
	char * aux;

	if ((aux=SearchExecutable(words[0]))==NULL) {
		printf ("%s not found\n",words[0]);
		return;
	}
	CheckPriorityChange (words+1);

	if (execv(aux,words)==-1)
		perror ("cannot execute");

}

void Cmd_Prog(char *words[]) 
{
	char * aux;
	pid_t pid;

	if (words[0]==NULL) return;
	if ((aux=SearchExecutable(words[0]))==NULL) {
		printf ("%s not found\n",words[0]);
		return;
	}
	CheckPriorityChange (words+1);

	if (pid=vfork()==0) {

		if (execv(aux,words)==-1)
			perror ("cannot execute");
	}	
	else  waitpid(pid,NULL,0);
}

char* CutWords (char * words[],int number) {
	int length=1;

	for (int i=0; i<number;i++)
		length+=strlen(words[i]) +1;

	char * returner = malloc(length*sizeof(char));
	strcpy(returner,"");

	for (int i=0; i < number; i++) 
	{
		strcat(returner,words[i]);
		strcat(returner, " ");	
	}		
	return returner;
}

void Cmd_Background(char *words[],int number) 
{
	char * aux;
	char * String;
	pid_t pid;

	if (words[0]==NULL) return;

	if ((aux=SearchExecutable(words[0]))==NULL) {
		printf ("%s not found\n",words[0]);
		return;
	}
	String=CutWords(words,number);
	CheckPriorityChange (words+1);


	if ((pid=vfork())==0) {
		if (execv(aux,words)==-1)
			perror ("cannot execute");
	}
	else if (pid>0){	

		time_t mytime;
		time(&mytime);
		item temp;

		temp.pid=pid;
		temp.prio=getpriority(PRIO_PROCESS,0);
		temp.sigName="ACTIVE";
		temp.time=mytime;
		temp.name=String;

		Insert(&ProL,temp);
		}	 
}
void Cmd_Jobs() {

	PrintList(&ProL);
}
void Cmd_Proc(char *words[]) {
	if (words[0]==NULL)
		Cmd_Jobs();
	else	
		PrintListPid(&ProL,atoi(words[0]));
}
void Cmd_ClearJobs() {
	DeleteAll(&ProL);
}





void ProcessInput(char *words[],int counter)
{
	if((strcmp(words[0],"fin")==0)||(strcmp(words[0],"end")==0)||strcmp(words[0],"exit")==0)
		exit(0);
	else if (strcmp(words[0],"autores")==0) Cmd_Autores(words+1);
	else if (strcmp(words[0],"pid")==0) Cmd_Pid(words+1);
	else if (strcmp(words[0],"help")==0) Cmd_Help();
	else if (strcmp(words[0],"info")==0) Cmd_Info(words+1,counter-1);
	else if (strcmp(words[0],"recursive")==0) Cmd_Recursive(words+1);
	else if (strcmp(words[0],"list")==0) Cmd_List(words+1);
	else if (strcmp(words[0],"eliminate")==0) Cmd_Eliminate(words+1);
	else if (strcmp(words[0],"malloc")==0) Cmd_Malloc(words+1);
	else if (strcmp(words[0],"mmap")==0) Cmd_Mmap(words+1);
	else if (strcmp(words[0],"sharednew")==0) Cmd_Sharednew(words+1);
	else if (strcmp(words[0],"shared")==0) Cmd_Shared(words+1);
	else if (strcmp(words[0],"rmkey")==0) Cmd_rmkey(words+1);
	else if (strcmp(words[0],"allocation")==0) Cmd_Allocation();
	else if (strcmp(words[0],"deallocate")==0) Cmd_Deallocate(words+1);
	else if (strcmp(words[0],"mem")==0) Cmd_Mem();
	else if (strcmp(words[0],"memdump")==0) Cmd_memdump(words+1);
	else if (strcmp(words[0],"recursivefunction")==0) Cmd_recursivefunction(words+1);
	else if (strcmp(words[0],"read")==0) Cmd_Read(words+1);
	else if (strcmp(words[0],"write")==0) Cmd_Write(words+1);
	else if (strcmp(words[0],"setpriority")==0) Cmd_SetPriority(words+1);
	else if (strcmp(words[0],"fork")==0) Cmd_Fork();
	else if (strcmp(words[0],"searchlist")==0) Cmd_SearchList(words+1);
	else if (strcmp(words[0],"exec")==0) Cmd_Exec(words+1);
	else if (strcmp(words[0],"background")==0) Cmd_Background(words+1,counter-1);
	else if (strcmp(words[0],"jobs")==0) Cmd_Jobs();
	else if (strcmp(words[0],"clearjobs")==0) Cmd_ClearJobs();
	else if (strcmp(words[0],"proc")==0) Cmd_Proc(words+1);
	else   Cmd_Prog(words);
}
int main()
{

	char s[MAX];
	char *pieces[MAX/2];
	int counter;
	CreateEmptyList(&ProL);
	printf("%s","************************************************************************* \n");
	printf("\t\t\t Shell Operating Systems \n");
	printf("%s\n","*************************************************************************\n");
	SearchListNew();

	while (1){
		printf ("->");
		fgets(s,MAX,stdin);
		counter=TrocearCadena(s,pieces);
		if (pieces[0]==NULL)
			continue;
		ProcessInput(pieces,counter);
	}
	return 0;
}




