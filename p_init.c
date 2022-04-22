#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

const int sizebuf = 16;
int run = 1;
pid_t p[4];

struct sembuf bufor;

void validateSem(int v);
void handlerS1(int sig);
void handlerS2(int sig);
void handlerS3(int sig);

int main(){ // init program
	
	int i;
			
	// Shared memory declaration
	int shmid = shmget(45286, sizebuf*sizeof(char), IPC_CREAT|0600);
	if(shmid == -1){
		perror("Creating shared memory error");
		exit(1);
	}
	// Loading a pointer to the shared memory
	void *bufor = shmat(shmid, NULL, 0);
	if(bufor == NULL){
		perror("Linking shared memory error");
		exit(1);
	}
	
	// Semaphores declaration
	int semid = semget(45288, 4, IPC_CREAT|0600);
    if (semid == -1)
    {
      perror("Creating semaphore array error");
      exit(1);
    }

    validateSem(semctl(semid, 0, SETVAL, (int)1));
    validateSem(semctl(semid, 1, SETVAL, (int)0));
    validateSem(semctl(semid, 2, SETVAL, (int)0));
    validateSem(semctl(semid, 3, SETVAL, (int)1));
		
	char pids[8];
	
	p[0] = getpid();
	snprintf(pids, sizeof(pids), "%d", p[0]);
	
	if((p[1] = fork()) == 0){
		execlp("./proces_1", "k2", pids, (char *)NULL);
	}
	else if((p[2] = fork()) == 0){
		execlp("./proces_2", "k2", pids, (char *)NULL);
	}
	else if((p[3] = fork()) == 0){ 
		execlp("./proces_3", "k3", pids, (char *)NULL);
	}
	else{
		printf("Init process: %d\n", getpid());
		for(i = 1;i < 4; i++) 
			printf("Process<%d>: %d\n", i, p[i]);
			
		sigset_t maskM;
		sigemptyset(&maskM);
		sigprocmask(SIG_SETMASK, &maskM, NULL);
	
		while(run){
			sleep(1);
			signal(SIGBUS, handlerS1);
			signal(SIGSTKFLT, handlerS2);
			signal(SIGCHLD, handlerS3);
		}
		
		for(i = 1;i < 4; i++){
			kill(p[i], SIGINT);
		}
		
		shmdt(bufor);
		shmctl(shmid, IPC_RMID, NULL);
		semctl(semid, 1, IPC_RMID);
		
	}
	
	
	return 0;
}

void validateSem(int v)
{
  if (v == -1)
  {
    perror("Error");
    exit(1);
  }
};

void handlerS1(int sig)
{

	printf("\n End \n");
	run = 0;

}

void handlerS2(int sig)
{
	int i;
	for(i = 1;i < 4; i++) kill(p[i], SIGSTKFLT);

}

void handlerS3(int sig)
{

	int i;
	for(i = 1;i < 4; i++) kill(p[i], SIGCHLD);

}
