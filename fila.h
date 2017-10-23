typedef struct fila Fila;
typedef struct no No;

Fila* fila_cria (int t);
void fila_insere (Fila* f, void* obj);
void * fila_retira (Fila* f);
int fila_vazia (Fila* f);
void fila_libera (Fila* f); 
int fila_tempo (Fila* f);
void* fila_obtemvalor(Fila *f);