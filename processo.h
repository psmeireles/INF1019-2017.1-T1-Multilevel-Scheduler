
typedef enum estado{
	novo,
	pronto,
	esperando,
	executando,
	terminado
} Estado;

typedef struct Processo{

//	int num_rajadas;
	int rajadas[50];
//	int tempo_restante;
	int rajadas_restantes;
//	char nome[50];
	int fila;
	int prox_fila;
	Estado estado;
	int pid;
} Processo;
