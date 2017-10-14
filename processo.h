typedef struct Processo{

	int num_rajadas;
	int rajadas[50];
	int tempo_restante;
	int rajadas_restantes;
	int fila_atual;
	char* nome;
	int pid;
} Processo;