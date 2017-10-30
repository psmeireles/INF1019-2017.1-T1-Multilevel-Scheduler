#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

/* Um processo recebe como parâmetros os tempos de rajada*/

int main(int argc, char *argv[]){
    int i, j;
    for(i = 0; i < argc; i++){
        for(j = 0; j < atoi(argv[i]); j++){
            printf("%d\n", getpid());
            sleep(1);
        }
        if(i != argc-1){
            kill(getppid(), SIGUSR2); // Avisou que entrou em I/O
            kill(getpid(), SIGSTOP);
        }
    }
}
