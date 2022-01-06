#include "abb.h"
#include "pila.h"

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct nodo {
    char* clave;
    void* dato;
    struct nodo* izq;
    struct nodo* der;
} nodo_t;

struct abb {
    nodo_t* raiz;
    size_t cantidad;
    abb_comparar_clave_t funcion_de_comparacion;
    abb_destruir_dato_t funcion_de_destruccion;
};

nodo_t *crear_nodo_abb(const char *clave, void *dato) {
    nodo_t *nodo = malloc(sizeof(nodo_t));
    if (!nodo) {
        return NULL;
    }
    nodo->clave = strdup(clave);
    nodo->dato = dato;
    nodo->izq = NULL;
    nodo->der = NULL;
    return nodo;   
}

abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato) {
    abb_t *arbol = malloc(sizeof(abb_t));
    if (arbol == NULL) {
        return NULL;
    }
    if (cmp != NULL) {
        arbol->funcion_de_comparacion = cmp;
    } else {
        arbol->funcion_de_comparacion = NULL;    
    }
    if (destruir_dato != NULL) {
        arbol->funcion_de_destruccion = destruir_dato;
    } else {
        arbol->funcion_de_destruccion = NULL;    
    }
    arbol->raiz = NULL;
    arbol->cantidad = 0;
    return arbol;
}

nodo_t** buscar_lugar(const abb_t *arbol, const char *clave, nodo_t **nodo_aux) {
    if (!*nodo_aux) {
        return nodo_aux;
    } else if (arbol->funcion_de_comparacion(clave, (*nodo_aux)->clave) > 0) {
        return buscar_lugar(arbol, clave, &((*nodo_aux)->der));
    } else if (arbol->funcion_de_comparacion(clave, (*nodo_aux)->clave) < 0) {
        return buscar_lugar(arbol, clave, &((*nodo_aux)->izq));
    }
    return nodo_aux;
}

bool abb_guardar(abb_t *arbol, const char *clave, void *dato) {
    nodo_t *nodo;
    nodo_t **p = buscar_lugar(arbol, clave, &(arbol->raiz));
    if ((*p) == NULL) {
    //LA CLAVE NO ESTA EN EL ARBOL
        nodo = crear_nodo_abb(clave, dato);
        if (!nodo) {
            return false;
        }
        *p = nodo;
        arbol->cantidad++;
    } else {
    //LA CLAVE ESTA EN EL ARBOL
        if (arbol->funcion_de_destruccion != NULL) {
            arbol->funcion_de_destruccion((*p)->dato);
        }
        (*p)->dato = dato;
    }
    return true;
}

nodo_t *buscar_remplazo(nodo_t *nodo) {
    nodo_t *remplazo = nodo->izq;
    while (remplazo->der != NULL) {
        remplazo = remplazo->der;
    }
    return remplazo;
}

void *abb_borrar(abb_t *arbol, const char *clave) {
    nodo_t **p = buscar_lugar(arbol, clave, &(arbol->raiz));
    if (*p == NULL) {
        return NULL;
    }
    void *dato = (*p)->dato;
    
    // borrado no tiene hijos //
    if (!(*p)->der && !(*p)->izq) {
        free((*p)->clave);
        free(*p);
        *p = NULL;
        arbol->cantidad--;
    // borrado tiene 1 hijo // 
    } else if ((!(*p)->der && (*p)->izq != NULL) || ((*p)->der != NULL && !(*p)->izq)) {
        char *clave_aux = (*p)->clave;
        nodo_t *nodo_aux = *p;
        if (!(*p)->izq) {
            *p = ((*p)->der);
        } else {
            *p = ((*p)->izq);
        }
        free(clave_aux);
        free(nodo_aux);
        arbol->cantidad--;
    // borrado tiene 2 hijos //
    } else {
        nodo_t *remplazo = buscar_remplazo(*p);
        char* clave_aux = strdup(remplazo->clave);
        void* dato_aux = abb_borrar(arbol, remplazo->clave);
        free((*p)->clave);
        (*p)->dato = dato_aux;
        (*p)->clave = clave_aux;
    }
    return dato;
}

void *abb_obtener(abb_t *arbol, const char *clave) {
    nodo_t **p = buscar_lugar(arbol, clave, &(arbol->raiz));
    if (*p == NULL) {
        return NULL;
    }
    return (*p)->dato;
} 

bool abb_pertenece(abb_t *arbol, const char *clave) {
    nodo_t **p = buscar_lugar(arbol, clave, &(arbol->raiz));
    return (*p) != NULL;
}

size_t abb_cantidad(abb_t *arbol) {
    return arbol->cantidad;
}

void nodos_destruir(abb_t *arbol, nodo_t *nodo) {
    if (nodo != NULL) {
        nodos_destruir(arbol, nodo->der);
        nodos_destruir(arbol, nodo->izq);
        if (arbol->funcion_de_destruccion != NULL) {
            arbol->funcion_de_destruccion(nodo->dato);
        }
        free(nodo->clave);
        free(nodo);     
    }
}

void abb_destruir(abb_t *arbol) {
    nodos_destruir(arbol, arbol->raiz);    
    free(arbol);   
}

//------------------------------------------------------------

void nodos_in_order(nodo_t *nodo, bool visitar(const char *, void *, void *), void *extra, bool *continuar) {
    if (nodo != NULL) {
        nodos_in_order(nodo->izq, visitar, extra, continuar);
        if (!*continuar) {
            return;
        }
        *continuar = visitar(nodo->clave, nodo->dato, extra);
        if (*continuar) {
            nodos_in_order(nodo->der, visitar, extra, continuar);
        }
    }
}

void abb_in_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra) {
    bool continuar = true;
    nodos_in_order(arbol->raiz, visitar, extra, &continuar);    
}

//------------------------------------------------------------

typedef pila_t pilanodos_t;

pilanodos_t *pilanodos_crear(void) {
    return pila_crear();
}

void apilar_nodo(pilanodos_t *pila, nodo_t *nodo) {
    pila_apilar(pila, nodo);
}

bool desapilar_nodo(pilanodos_t *pila) {
    if (pila_esta_vacia(pila)) {
        return false;
    }
    pila_desapilar(pila);
    return true;
}

bool pilanodo_esta_vacia(pilanodos_t *pila) {
    return pila_esta_vacia(pila);
}

void pilanodo_destruir(pilanodos_t *pila) {
    pila_destruir(pila);
}

nodo_t *pilanodo_ver_tope(const pilanodos_t *pila) {
    nodo_t *nodo = (nodo_t *)pila_ver_tope(pila);
    return nodo;
}

struct abb_iter {
    pilanodos_t *pila;   
};

abb_iter_t *abb_iter_in_crear(const abb_t *arbol) {
    abb_iter_t *iter = malloc(sizeof(abb_iter_t));
    if (!iter) {
        return NULL;
    }
    iter->pila = pilanodos_crear();
    if (arbol->raiz != NULL) {
        nodo_t *nodo = arbol->raiz;
        apilar_nodo(iter->pila, nodo);
        while (nodo->izq != NULL) {
            nodo = nodo->izq;
            apilar_nodo(iter->pila, nodo);
        }
    }
    return iter;
}

bool abb_iter_in_avanzar(abb_iter_t *iter) {
    if (abb_iter_in_al_final(iter)) {
        return false;
    }
    nodo_t *nodo = pilanodo_ver_tope(iter->pila)->der;
    desapilar_nodo(iter->pila);
    if (nodo != NULL) {
        apilar_nodo(iter->pila, nodo);
        while (nodo->izq != NULL) {
            nodo = nodo->izq;
            apilar_nodo(iter->pila, nodo);
        }
    }
    return true;
}

const char *abb_iter_in_ver_actual(const abb_iter_t *iter) {
    nodo_t *nodo = pilanodo_ver_tope(iter->pila);
    if (nodo != NULL) {
        return nodo->clave;
    }
    return NULL;
}

bool abb_iter_in_al_final(const abb_iter_t *iter) {
    return pilanodo_esta_vacia(iter->pila);
}

void abb_iter_in_destruir(abb_iter_t* iter) {
    pilanodo_destruir(iter->pila);
    free(iter);
}
