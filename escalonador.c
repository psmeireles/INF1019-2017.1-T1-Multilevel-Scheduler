#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include "processo.h"
#include "fila.h"


Processo *pcorrente, *pIO;
pid_t pid_escalonador;
Fila *level1, *level2, *level3;

int flag_IO = 0, flag_Child = 0;

int RoundRobin(Fila *f);

int ExecutarProcesso(Processo *p, int t);

char** Preenche_argv(Processo *p);

void IOHandler(int sinal);

void ChildHandler(int sinal);

void alarmHandler(int sinal);



int main(){
	char **argv;
    int i = 0;
    Processo *p1 = (Processo *) malloc(sizeof(Processo)), *p2 = (Processo *) malloc(sizeof(Processo));
    
    level1 = fila_cria(1);
    level2 = fila_cria(2);
    level3 = fila_cria(4);
    
    signal(SIGALRM, alarmHandler);
    signal(SIGUSR2, IOHandler);		// Começa a escutar sinal de que entrou em I/O
    signal(SIGCHLD, ChildHandler);	// Começa a escutar sinal de que processo terminou
	
	pid_escalonador = getpid();
	
	p1->rajadas[0] = 10;
	p1->rajadas[1] = 1;
	p1->rajadas[2] = 1;
	p1->rajadas[3] = 0;
	p1->fila = 1;
	p1->prox_fila = 2;
    p1->estado = novo;
    
    p2->rajadas[0] = 4;
	p2->rajadas[1] = 3;
	p2->rajadas[2] = 7;
	p2->rajadas[3] = 1;
	p2->rajadas[4] = 0;
	p2->fila = 1;
	p2->prox_fila = 2;
    p2->estado = novo;
    
    fila_insere(level1, p1);
    fila_insere(level1, p2);
	
    while(1){
        RoundRobin(level1);
        RoundRobin(level2);
        RoundRobin(level3);
    }
	
    return 0;
}


int RoundRobin(Fila *f){    // Executa o escalonamento Round Robin da fila recebida
	int ret;
	
    while(!fila_vazia(f)){
        pcorrente = fila_retira(f);
        ret = ExecutarProcesso(pcorrente, fila_tempo(f));
        // TODO: tratar os retornos da ExecutarProcesso
        if(ret == 0){   // Processo usou todo o quantum
			if(f == level1){
				pcorrente->fila = 2;
				fila_insere(level2, pcorrente);
			} 
			else if(f == level2 || f == level3){
				pcorrente->fila = 3;			
				fila_insere(level3, pcorrente);
			}
        }
		else if(ret == 1){  // Processo entrou em I/O
			/* TODO: Preciso saber se tenho que retornar algo avisando que tem que voltar pra fila de cima ou se continuo executando */
        }
        else{   // Processo terminou
			//free(pcorrente); -> Aqui deu um erro monstro
			// Só isso?
        }
    }
    return 0;
}


int ExecutarProcesso(Processo *p, int t){    // Executa um novo processo ou continua o antigo
/* Retorna 1 se o processo terminou em I/O
   Retorna 2 se o processo terminou de executar
   Retorna 0 se o processo esgotou o tempo de processamento sem terminar */
	int i;
    pid_t pid_novo;
    sigset_t mask, oldmask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGCHLD);

	printf("L%d\n", p->fila);

    if(p->estado == pronto){    // Se o processo não é novo, só manda o sinal para continuar
        sigprocmask (SIG_BLOCK, &mask, &oldmask);
        kill(p->pid, SIGCONT);
        sleep(t);
        kill(p->pid, SIGSTOP);
        sigprocmask (SIG_UNBLOCK, &mask, NULL);
        if(flag_IO == 1){ // Entrou em I/O
            printf("Flag_IO = 1\n");
            alarm(3);
            flag_IO = 0;
            p->estado = esperando;
            return 1;
        }
        if(flag_Child == 1){	// Terminou de executar
            printf("Flag_Child = 1\n");
            flag_Child = 0;
            p->estado = terminado;
            return 2;
        }
        p->estado = pronto;
        return 0;
    }
    else if(p->estado == novo){   // Se é um processo novo, faz um fork()
        if( (p->pid = fork()) == 0 ){    // Filho
            char **argv;
            int i = 0;
            argv = Preenche_argv(p);    // Preenche o vetor com os argumentos a serem passados para o processo a ser executado
            if(execve("./prog1", argv, NULL) == -1){    // Executa o processo
                printf("Erro no execv\n");
                exit(1);
            }
        }
        else{   // Pai
            sigprocmask (SIG_BLOCK, &mask, &oldmask);
            sleep(t);
            kill(p->pid, SIGSTOP);
            sigprocmask (SIG_UNBLOCK, &mask, NULL);
            if(flag_IO == 1){	// Entrou em I/O
                printf("Flag_IO = 1\n");
                alarm(3);
                flag_IO = 0;
                p->estado = esperando;
                return 1;
            }
            if(flag_Child == 1){	// Terminou de executar
                printf("Flag_Child = 1\n");
                flag_Child = 0;
                p->estado = terminado;
                return 2;
            }
            p->estado = pronto;
            return 0;
        }
    }
}

// Testada e funcionando
char** Preenche_argv(Processo *p){  // Preenche argv com os parâmetros relativos ao processor recebido
    char **argv;
    int nrajadas = 0, i = 0;

    while(p->rajadas[i] != 0) {
    	nrajadas++;
    	i++;
    }   // Conta o número de rajadas

    argv = (char**) malloc((nrajadas+1)*sizeof(char*));    // Aloca com tamanho suficiente para o número de rajadas + 2
    for(i = 0; i < nrajadas + 1; i++) {
        argv[i] = (char*) malloc(30*sizeof(char));
    }
    
    for(i = 0; i < nrajadas; i++){              // A partir do terceiro argumento vêm os tamanhos das rajadas
        sprintf(argv[i], "%d", p->rajadas[i]);
    }
	argv[i] = NULL;
	
    return argv;
}

void IOHandler(int sinal){
	kill(pcorrente->pid, SIGSTOP);
    printf("Filho entrou em I/O\n");
	pcorrente->prox_fila = pcorrente->fila - 1;
	if(pcorrente->prox_fila == 0)
		pcorrente->prox_fila = 1;
	pIO = pcorrente;
    flag_IO = 1;
    // TODO: Descobrir como fazer com que os alarmes ocorram em paralelo
}

void ChildHandler(int sinal){
	int pid_encerrado = waitpid(-1, NULL, WNOHANG);
	if(pid_encerrado == 0){ // SIGSTOP ou SIGCONT
		return;
	}
    printf("Filho mandou SIGCHLD\n");
    flag_Child = 1;
    // free(pcorrente); -> acho que aqui é o melhor lugar pro free
}

// Promissor, mas ainda precisa de mais testes
void alarmHandler(int sinal){
    signal(SIGALRM, SIG_IGN);
    pIO->estado = pronto;
	if(pIO->prox_fila == 2){
		pIO->fila = 2;
		fila_insere(level2, pIO);
	}
	else{
		pIO->fila = 1;
		fila_insere(level1, pIO);
    }
    signal(SIGALRM, alarmHandler);
	// Como fazer pro alarme saber em que fila ele tem que botar? Acho que resolvi
}
