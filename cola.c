#include "cola.h"

#include <stdbool.h>
#include <stdlib.h>

typedef struct nodo {
    void* dato;
    struct nodo* prox;
}nodo_t;

struct cola {
    nodo_t* prim;
    nodo_t* ult;
};

nodo_t* crear_nodo_cola(void* valor) {
    nodo_t* nodo = malloc(sizeof(nodo_t));
    if(!nodo) {
        return NULL;
    }
    nodo->dato = valor;
    nodo->prox = NULL;
    return nodo;
}

cola_t *cola_crear(void) {
    cola_t* cola = malloc(sizeof(cola_t));
    if(!cola) {
        return NULL;
    }
    cola->prim = NULL;
    cola->ult = NULL;
    return cola;
}

void cola_destruir(cola_t *cola, void (*destruir_dato)(void *)) {
    while (!cola_esta_vacia(cola)) {
        nodo_t* nodo = cola->prim;
        if (destruir_dato) {
            destruir_dato(nodo->dato);
        }
        cola->prim = cola->prim->prox;
        free(nodo);
    }
    free(cola);
}

bool cola_esta_vacia(const cola_t *cola) {
    if (!cola->prim) {
        return true;
    }
    return false;
}

bool cola_encolar(cola_t *cola, void *valor) {
    nodo_t* nodo = crear_nodo_cola(valor);
    if (!nodo) {
        return false;
    }
    if (cola_esta_vacia(cola)) {
        cola->prim = nodo;
    } else {
        cola->ult->prox = nodo;
    }
    cola->ult = nodo;
    return true;  
}

void *cola_ver_primero(const cola_t *cola) {
    if (cola_esta_vacia(cola)) {
        return NULL;
    }
    return cola->prim->dato;
}

void *cola_desencolar(cola_t *cola) {
    if (cola_esta_vacia(cola)) {
        return NULL;
    }
    void* dato = cola->prim->dato;
    nodo_t* nodo = cola->prim;
    cola->prim = cola->prim->prox;
    free(nodo);
    return dato;
}
