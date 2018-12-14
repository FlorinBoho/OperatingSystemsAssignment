#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "listproc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>




struct SEN{
char *nombre;
int senal;
};
static struct SEN sigstrnum[]={
		"HUP", SIGHUP,
		"INT", SIGINT,
		"QUIT", SIGQUIT,
		"ILL", SIGILL,
		"TRAP", SIGTRAP,
		"ABRT", SIGABRT,
		"IOT", SIGIOT,
		"BUS", SIGBUS,
		"FPE", SIGFPE,
		"KILL", SIGKILL,
		"USR1", SIGUSR1,
		"SEGV", SIGSEGV,
		"USR2", SIGUSR2,
		"PIPE", SIGPIPE,
		"ALRM", SIGALRM,
		"TERM", SIGTERM,
		"CHLD", SIGCHLD,
		"CONT", SIGCONT,
		"STOP", SIGSTOP,
		"TSTP", SIGTSTP,
		"TTIN", SIGTTIN,
		"TTOU", SIGTTOU,
		"URG", SIGURG,
		"XCPU", SIGXCPU,
		"XFSZ", SIGXFSZ,
		"VTALRM", SIGVTALRM,
		"PROF", SIGPROF,
		"WINCH", SIGWINCH,
		"IO", SIGIO,
		"SYS", SIGSYS,
		/*senales que no hay en todas partes*/
		#ifdef SIGPOLL
		"POLL", SIGPOLL,
		#endif
		#ifdef SIGPWR
		"PWR", SIGPWR,
		#endif
		#ifdef SIGEMT
		"EMT", SIGEMT,
		#endif
		#ifdef SIGINFO
		"INFO", SIGINFO,
		#endif
		#ifdef SIGSTKFLT
		"STKFLT", SIGSTKFLT,
		#endif
		#ifdef SIGCLD
		"CLD", SIGCLD,
		#endif
		#ifdef SIGLOST
		"LOST", SIGLOST,
		#endif
		#ifdef SIGCANCEL
		"CANCEL", SIGCANCEL,
		#endif
		#ifdef SIGTHAW
		"THAW", SIGTHAW,
		#endif
		#ifdef SIGFREEZE
		"FREEZE", SIGFREEZE,
		#endif
		#ifdef SIGLWP
		"LWP", SIGLWP,
		#endif
		#ifdef SIGWAITING
		"WAITING", SIGWAITING,
		#endif
		NULL,-1,
}; /*fin array sigstrnum */
int Senal(char * sen) /*devuel el numero de senial a partir del nombre*/
{
	int i;
	for (i=0; sigstrnum[i].nombre!=NULL; i++)
	if (!strcmp(sen, sigstrnum[i].nombre))
	return sigstrnum[i].senal;
	return -1;
	
}
char * NombreSenal(int sen) /*devuelve el nombre senal a partir de la senal*/
{ /* para sitios donde no hay sig2str*/
	int i;
	for (i=0; sigstrnum[i].nombre!=NULL; i++)
	if (sen==sigstrnum[i].senal)
	return sigstrnum[i].nombre;
	return ("SIGUNKNOWN");
}


void CreateEmptyList(Prolist *l) {
	
	l->lastPos=-1;
	
}
int Insert(Prolist *l,item aux) {

	
	
	if (l->lastPos == MAXSIZE-1) 
			return -1;
	
	l->lastPos++;		
	
	
	l->data[l->lastPos]=aux;
	return 1;

}

void PrintDetails(item aux) {
	
printf("%5d  %10s ",aux.pid,aux.sigName);
		if ( (strcmp(aux.sigName,"STOPPED")==0) || (strcmp(aux.sigName,"SIGNALED")==0)) {
			printf("(%s) ",aux.signal);
		}
		else if (strcmp(aux.sigName,"TERMINATED")==0){
				printf("(%d) ",aux.exitcode);
			}
			else
				printf("%5s","");
		
		printf("p=%d  %.19s %s \n",aux.prio,ctime(&aux.time),aux.name);
	}
void UpdateState(item * aux) {
	int status;
	
	if ( (aux->pid==waitpid(aux->pid,&status,WNOHANG | WUNTRACED |WCONTINUED))) 
		{
				
				if (WIFEXITED(status)!=0){
					aux->sigName="TERMINATED";
					aux->exitcode=WEXITSTATUS(status);
				}
				else if (WIFSIGNALED(status)) {
						 aux->sigName="SIGNALED";
						aux->signal=NombreSenal(WTERMSIG(status));
				      }
					 else	if (WIFSTOPPED(status)) {
								aux->sigName="STOPPED";
								aux->signal=NombreSenal(WSTOPSIG(status));
						} 
						else if (WIFCONTINUED(status))
								aux->sigName="ACTIVE";
					
						
		}

}
void PrintList(Prolist *  l) {
	
	
	int test=0;
	for (int i=0; i<=l->lastPos;i++){ 

		
	
		UpdateState(&(l->data[i]));
		PrintDetails(l->data[i]);
		
	}
}

void PrintListPid(Prolist * l, pid_t pid){
	for (int i=0; i<=l->lastPos;i++) {
		if (pid == l->data[i].pid) {
			UpdateState(&(l->data[i]));
			PrintDetails(l->data[i]);
			return;
		}
	}
		PrintList(l);
		
		
}		
void DeleteAll(Prolist *l){
	l->lastPos=-1;
}
