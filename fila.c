#include<stdlib.h>
#include"fila.h"
#include<stdio.h>

struct no{
	void* obj;
	No *prox;
};

struct fila{
    No *ini;
    int tempo;
};

Fila * fila_cria(int t){
	Fila *f;
	f = (Fila *) malloc(sizeof(Fila));
	if(f == NULL){
		printf("Falta de memoria.\n");
		exit(1);
	}
    f->ini = NULL;
    f->tempo = t;
	return f;
}

void fila_insere(Fila *f, void* obj){
	No *p, *novo;
	if(f->ini == NULL){
		novo = (No *) malloc(sizeof(No));
		novo->obj = obj;
		novo->prox = NULL;
		f->ini = novo;
	}
	else{
		p = f->ini;
		while(p->prox != NULL)
			p = p->prox;
		novo = (No *) malloc(sizeof(No));
		novo->obj = obj;
		novo->prox = NULL;
		p->prox = novo;
	}
}

void * fila_retira(Fila *f){
	void* saida;
	No *aux;
	aux = f->ini;
	saida = aux->obj;
	f->ini = aux->prox;
	free(aux);
	return saida;
}

int fila_vazia(Fila *f){
	if(f->ini == NULL)
		return 1;
	else return 0;
}

void fila_libera(Fila *f){
	No *p, *aux;
	p = f->ini;
	while(p != NULL){
		aux = p;
		p = aux->prox;
		free(aux);
	}
	free(f);
}

int fila_tempo(Fila *f){
    return f->tempo;
}

void * fila_obtemvalor(Fila *f){
    return f->ini->obj;
}