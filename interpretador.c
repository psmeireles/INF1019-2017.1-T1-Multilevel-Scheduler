#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "processo.h"

void quitHandler(int sinal);

int main(){

	Processo* processo1;
	processo1 = (Processo*)malloc(sizeof(Processo));
	int segmento1, segmento2;
	Processo *p;
	int *pid;
	segmento1 = shmget(8778, sizeof(Processo), IPC_CREAT | IPC_EXCL | S_IRWXU);
	p = (Processo*)shmat(segmento1, 0, 0);

	char comando[50];
	char temp1[50];
	char *temp2;
	int numTemp;
	int j = 0;

	segmento2 = shmget(8779, sizeof(int), IPC_CREAT | S_IRWXU);
	pid = (int*)shmat(segmento2, 0, 0);

	signal(SIGQUIT, quitHandler);
	signal(SIGINT, quitHandler);

	while(1){

		j = 0;

		scanf("%50[^\n]", comando);
		while ( getchar() != '\n' );

		strcpy(temp1, comando);

		temp2 = strtok(temp1, " ");
		temp2 = strtok(NULL, "(");

		//strcpy(processo1->nome, temp2);

		temp2 = strtok(NULL, ",");
		while(temp2 != NULL){
			numTemp = atoi(temp2);
			processo1->rajadas[j] = numTemp;
			temp2 = strtok(NULL, ",");
			j++;
		}
		while(j <= 49){
			processo1->rajadas[j] = 0;
			j++;
		}

		processo1->fila = 1;
		processo1->prox_fila = 2;
		processo1->estado = novo;
		processo1->pid = 1;

		*p = *processo1;

		printf("\n%d\n", *pid);

		kill(*pid, SIGUSR1);
	}
	shmdt(p);
}

void quitHandler(int sinal)
{
	int segmento;
	segmento = shmget(8778, 0, S_IRWXU);
	shmctl(segmento, IPC_RMID, 0);

	segmento = shmget(8779, 0, S_IRWXU);
	shmctl(segmento, IPC_RMID, 0);

	printf("Terminando...\n");
	exit (0);
}










































