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
Fila *level1, *level2, *level3, *io;

int flag_IO = 0, flag_Child = 0;

void RoundRobin(Fila *f);

int ExecutarProcesso(Processo *p, int t);

char** Preenche_argv(Processo *p);

void IOHandler(int sinal);

void ChildHandler(int sinal);

void alarmHandler(int sinal);

void newProcessHandler(int signal);

void quitHandler(int signal);


int main(){
    int segmento, *pid;
    
    level1 = fila_cria(1);
    level2 = fila_cria(2);
    level3 = fila_cria(4);
    io = fila_cria(3);
    
    signal(SIGALRM, alarmHandler);      // Sinal de alarme
    signal(SIGUSR2, IOHandler);         // Sinal de que entrou em I/O
    signal(SIGCHLD, ChildHandler);      // Sinal de que processo terminou
    signal(SIGUSR1, newProcessHandler); // Sinal de que chegou um novo processo
    signal(SIGQUIT, quitHandler);       // Sinais de fim do programa pelo teclado
    signal(SIGINT, quitHandler);

    pid_escalonador = getpid();

    segmento = shmget(8779, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRWXU);
    pid = (int*)shmat(segmento, 0, 0);

    *pid = pid_escalonador;
    
    while(1){
    	if(fila_vazia(level1) && fila_vazia(level2) && fila_vazia(level3)){
    		printf("Dormindo\n");
    		pause();
		}
        RoundRobin(level1);
        RoundRobin(level2);
        RoundRobin(level3);
    }
}


void RoundRobin(Fila *f){    // Executa o escalonamento Round Robin da fila recebida
    int ret;
    
    // Verifica se algum processo foi inserido em uma fila superior
    if(fila_tempo(f) == 2){
        if(!fila_vazia(level1)){
            return;
        }
    }
    else if(fila_tempo(f) == 4){
        if(!fila_vazia(level1) || !fila_vazia(level1)){
            return;
        }
    }
    
    while(!fila_vazia(f)){
        pcorrente = fila_retira(f);

        ret = ExecutarProcesso(pcorrente, fila_tempo(f));
        
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
            // Não há necessidade de tratar. Os sinais tratam sozinhos
        }
        else{   // Processo terminou
            free(pcorrente);
        }

        // Verifica se algum processo foi inserido em uma fila superior
        if(fila_tempo(f) == 2){
            if(!fila_vazia(level1)){
                return;
            }
        }
        else if(fila_tempo(f) == 4){
            if(!fila_vazia(level1) || !fila_vazia(level2)){
                return;
            }
        }
    }

    // Começa a exetuar a próxima fila
    return;
}


int ExecutarProcesso(Processo *p, int t){    // Executa um novo processo ou continua o antigo
/* Retorna 1 se o processo terminou em I/O
   Retorna 2 se o processo terminou de executar
   Retorna 0 se o processo usou todo o quantum */

    pid_t pid_temp;
    sigset_t mask, oldmask;

    // Determinando quais sinais serão ouvidos durante o sleep
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGCHLD);

    printf("\nL%d\n", p->fila);

    if(p->estado == pronto){    // Se o processo não é novo, só manda o sinal para continuar
        sigprocmask (SIG_BLOCK, &mask, &oldmask);
        kill(p->pid, SIGCONT);
        sleep(t);
        kill(p->pid, SIGSTOP);
        sigprocmask (SIG_UNBLOCK, &mask, NULL);

        if(flag_IO == 1){ // Entrou em I/O
            flag_IO = 0;
            p->estado = esperando;
            return 1;
        }

        if(flag_Child == 1){    // Terminou de executar
            flag_Child = 0;
            p->estado = terminado;
            return 2;
        }

        // Usou todo o quantum
        p->estado = pronto;
        return 0;
    }
    else{   // Se é um processo novo, faz um fork()
        if( (p->pid = fork()) == 0 ){    // Filho
            char **argv;
            argv = Preenche_argv(p);    // Preenche o vetor com os argumentos a serem passados para o processo a ser executado
            if(execve("./prog1", argv, NULL) == -1){    // Executa o processo
                printf("Erro no execv\n");
                exit(1);
            }
        }
        else{   // Pai
            pid_temp = p->pid;

            sigprocmask (SIG_BLOCK, &mask, &oldmask);
            sleep(t);
            kill(pid_temp, SIGSTOP);
            p->pid = pid_temp;
            sigprocmask (SIG_UNBLOCK, &mask, NULL);

            if(flag_IO == 1){   // Entrou em I/O
                flag_IO = 0;
                p->estado = esperando;
                return 1;
            }
            
            if(flag_Child == 1){    // Terminou de executar
                flag_Child = 0;
                p->estado = terminado;
                return 2;
            }
            
            // Usou todo o quantum
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
    }

    argv = (char**) malloc((nrajadas+1)*sizeof(char*));    // Aloca com tamanho suficiente para o número de rajadas + 1 (NULL)
    if(!argv){
        printf("Falta de memoria\n");
        exit(1);
    }
    for(i = 0; i < nrajadas + 1; i++) {
        argv[i] = (char*) malloc(sizeof(char));
        if(!argv[i]){
            printf("Falta de memoria\n");
            exit(1);
        }
    }
    
    for(i = 0; i < nrajadas; i++){              // A partir do terceiro argumento vêm os tamanhos das rajadas
        sprintf(argv[i], "%d", p->rajadas[i]);
    }
    argv[i] = NULL;
    
    return argv;
}

void IOHandler(int sinal){
    printf("%d entrou em I/O\n", pcorrente->pid);
    kill(pcorrente->pid, SIGSTOP);
    if(fork() == 0){ // Filho
        sleep(3);
        kill(getppid(), SIGALRM);
        kill(getpid(), SIGSTOP);
    }
    else{   // Pai
        pcorrente->prox_fila = pcorrente->fila - 1;
        if(pcorrente->prox_fila == 0)
            pcorrente->prox_fila = 1;
        fila_insere(io, pcorrente);
        flag_IO = 1;
    }
}

void ChildHandler(int sinal){
    int pid_encerrado = waitpid(-1, NULL, WNOHANG);
    if(pid_encerrado == 0){ // Ainda não acabou
        return;
    }
    printf("%d terminou\n", pid_encerrado);
    flag_Child = 1;
}

void alarmHandler(int sinal){
    signal(SIGALRM, SIG_IGN);
    pIO = fila_retira(io);
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
}

void newProcessHandler(int signal){

    int segmento, i;
    Processo *p, *novo = (Processo *) malloc (sizeof(Processo));
    if(!novo){
        printf("Falta de memoria\n");
        exit(1);
    }
    
    segmento = shmget(8778, sizeof(Processo), IPC_CREAT | S_IRWXU);
    p = (Processo*)shmat(segmento, 0, 0);

    for(i = 0; i < 50; i++)
        novo->rajadas[i] = p->rajadas[i];

    novo->fila = p->fila;
    novo->prox_fila = p->prox_fila;
    novo->estado = p->estado;
    novo->pid = p->pid;

    fila_insere(level1, novo);
}

void quitHandler(int sinal)
{
    int segmento;
    fila_libera(level1);
    fila_libera(level2);
    fila_libera(level3);
    fila_libera(io);

    segmento = shmget(8778, 0, S_IRWXU);
	shmctl(segmento, IPC_RMID, 0);

	segmento = shmget(8779, 0, S_IRWXU);
	shmctl(segmento, IPC_RMID, 0);

	printf("\nTerminando...\n");
	exit (0);
}
