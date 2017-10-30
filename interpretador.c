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
	int segmento1, segmento2;
	Processo *p;
	int *pid;
	char comando[50];
	char temp1[50];
	char *temp2;
	int numTemp;
	int j = 0;

	processo1 = (Processo*)malloc(sizeof(Processo));
	if(!processo1){
        printf("Falta de memoria\n");
        exit(1);
    }

	// Segmento de memória compartilhada onde o processo será escrito
	segmento1 = shmget(8778, sizeof(Processo), IPC_CREAT | IPC_EXCL | S_IRWXU);
	p = (Processo*)shmat(segmento1, 0, 0);	

	// Segmento de memória compartilhada com o pid do escalonador
	segmento2 = shmget(8779, sizeof(int), IPC_CREAT | S_IRWXU);
	pid = (int*)shmat(segmento2, 0, 0);

	// Sinais de fim do programa pelo teclado
	signal(SIGQUIT, quitHandler);
	signal(SIGINT, quitHandler);

	while(1){

		j = 0;

		scanf("%50[^\n]", comando);	// Lê o comando
		while ( getchar() != '\n' );

		// Pegando as rajadas
		strcpy(temp1, comando);

		temp2 = strtok(temp1, " ");
		temp2 = strtok(NULL, "(");

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

		// Informações padrão de um processo recém criado
		processo1->fila = 1;
		processo1->prox_fila = 2;
		processo1->estado = novo;
		processo1->pid = 1;

		// Guardando na memória compartilhada
		*p = *processo1;

		// Avisa ao escalonador que há um novo processo
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

	printf("\nTerminando...\n");
	exit (0);
}