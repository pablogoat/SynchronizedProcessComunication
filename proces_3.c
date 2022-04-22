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

pid_t parent;
int run = 1, work = 1, stop = 0, signal_rcv = 0, signal_snd = 0;
const int sizebuf = 16;
int semid;

struct sembuf bufor;

void setSem(int semid, int semnum, int opt);
void handlerS1(int sig);
void handlerS2(int sig);
void handlerS3(int sig);
void handlerS4(int sig);
void handlerS5(int sig);
void s_action();

int main(int argc, char *argv[]){
	
	//parent = atoi(argv[1]);
	parent = getppid();
	semid = semget(45288, 4, 0);
	if(semid == -1)
	{
		perror("Semaphores connection error - process 3");
		exit(1);
	}
	
	int shmid = shmget(45286, sizebuf*sizeof(char), 0);
	if(shmid == -1)
	{
		perror("Shared memory connection error - process 3");
		exit(1);
	}
	
	void *bufor;
	bufor = shmat(shmid, NULL, 0);
	if(bufor == NULL)
	{
		perror("Linking shared memory error - process 3");
		exit(1);
	}
	
	sigset_t maskM;
	sigemptyset(&maskM);
	sigprocmask(SIG_SETMASK, &maskM, NULL);
	signal(SIGBUS, handlerS1);
	signal(SIGSTKFLT, handlerS2);
	signal(SIGCHLD, handlerS3);
	signal(SIGCONT, handlerS4);
	signal(SIGPIPE, handlerS5);
	
	char cos[sizebuf];
	
	while(run)
	{
		if(work){
			setSem(semid, 2, -1);
			memcpy(cos, bufor, sizeof(cos));
			strcat(cos, "-3");
			printf("P3 - converted: %s\n", cos);
			setSem(semid, 0, 1);
			sleep(1);
		}
		
		s_action();
	}

	return 0;
}

void setSem(int semid, int semnum, int opt) {
  bufor.sem_num = semnum;
  bufor.sem_op = opt;
  bufor.sem_flg = 0;
  while (semop(semid, &bufor, 1) == -1) {
    if (errno == EINTR) {
      // signal interrupted sembuf
      //debug_print("EINTR\n");
    } else {
      perror("Failed to change semaphore value\n");
      exit(1);
    }
  };
};

void handlerS1(int sig)
{

	if(run){
		kill(parent, SIGBUS);
		run = 0;
	}
		
	return;
}

void handlerS2(int sig)
{
	
	if(work){
		kill(parent, SIGSTKFLT);
		work = 0;
	}
		
	return;
}

void handlerS3(int sig)
{
	
	if(work == 0){
		kill(parent, SIGCHLD);
		work = 1;
	}
		
	return;
}

void handlerS4(int sig)
{
	signal_rcv = 1;
	return;
}

void handlerS5(int sig)
{
	signal_rcv = 2;
	return;
}


void s_action()
{
	if(signal_snd == 1){
		kill(parent, SIGSTKFLT);
	}
	else if (signal_snd == 2) {
		kill(parent, SIGCHLD);
	}
	if(signal_rcv == 1){
		work = 0;
		printf("Wstrzymanie");
	}
	else if(signal_rcv == 2){
		work = 1;
		printf("Wznowienie");
	}
	signal_snd = 0;
	signal_rcv = 0;
	return;
}
