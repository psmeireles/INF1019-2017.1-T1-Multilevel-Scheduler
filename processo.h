
typedef enum estado{
	novo,
	pronto,
	esperando,
	executando,
	terminado
} Estado;

typedef struct Processo{
	int rajadas[50];
	int fila;
	int prox_fila;
	Estado estado;
	int pid;
} Processo;
