#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

/* Um processo recebe como parâmetros o pid do pai
e os tempos de rajada, nessa ordem */

int main(int argc, char *argv[]){
    int i, j;
    for(i = 0; i < argc; i++){
        for(j = 0; j < atoi(argv[i]); j++){
            printf("%daaaa\n", getpid());
            sleep(1);
        }
        if(i != argc-1){
            kill(getppid(), SIGUSR2); // Avisou que entrou em I/O
            // Esse sigstop é provisório. No futuro tenho que dar um jeito de sincronizar
		    printf("\n\%dbobmbmbjn", getpid());
            kill(getpid(), SIGSTOP);
        }
    }
}
