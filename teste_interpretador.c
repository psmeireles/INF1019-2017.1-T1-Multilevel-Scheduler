#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "processo.h"

int main(){
	int segmento;
	Processo *p;

	segmento = shmget(8766, sizeof(Processo), IPC_CREAT | S_IRWXU);
	p = (Processo*)shmat(segmento, 0, 0);

	//printf("\n%d\n",p->tempo_restante);

	printf("\n%d\n",p->rajadas_restantes);

	for(int i=0;i<p->rajadas_restantes;i++){
		printf("%d ", p->rajadas[i]);	
	}
}
