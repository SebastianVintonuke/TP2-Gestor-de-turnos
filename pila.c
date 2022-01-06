#include "pila.h"

#include <stdlib.h>

#define LARGO_INICIAL 10

/* Definición del struct pila proporcionado por la cátedra. */
struct pila {
    void **datos;
    size_t cantidad;   // Cantidad de elementos almacenados.
    size_t capacidad;  // Capacidad del arreglo 'datos'.
};

pila_t *pila_crear(void) {
    pila_t *pila = malloc(sizeof(pila_t));
    if (pila == NULL) {
        return NULL;
    }
    pila->capacidad = LARGO_INICIAL;
    pila->datos = malloc(pila->capacidad * sizeof(void *));
    if (pila->datos == NULL) {
        free(pila);
        return NULL;
    }
    pila->cantidad = 0;
    return pila;
}

void pila_destruir(pila_t *pila) {
    free(pila->datos);
    free(pila);
}

bool pila_esta_vacia(const pila_t *pila) {
    return pila->cantidad == 0;
}

bool pila_redimensionar(pila_t *pila, bool crecimiento) {
    if (crecimiento) {
        void **datos_nuevo = realloc(pila->datos, (pila->capacidad * 2) * sizeof(void *));
        if (datos_nuevo == NULL) {
            return false;
        }
        pila->datos = datos_nuevo;
        pila->capacidad = pila->capacidad * 2;
    }
    if (!crecimiento) {
        void **datos_nuevo = realloc(pila->datos, (pila->capacidad / 2) * sizeof(void *));
        if (datos_nuevo != NULL) {
            pila->datos = datos_nuevo;
            pila->capacidad = pila->capacidad / 2;
        }
    }
    return true;
}

bool pila_apilar(pila_t *pila, void *valor) {
    if (pila->cantidad == pila->capacidad) {
        if (!(pila_redimensionar(pila, true))) {
            return false;
        }
    }
    pila->datos[pila->cantidad] = valor;
    pila->cantidad++;
    return true;
}

void *pila_ver_tope(const pila_t *pila) {
    if (pila_esta_vacia(pila)) {
        return NULL;
    }
    return (pila->datos[pila->cantidad-1]);
}

void *pila_desapilar(pila_t *pila) {
    if (pila_esta_vacia(pila)) {
        return NULL;
    }
    if ((pila->cantidad * 4 <= pila->capacidad) && (pila->capacidad > 2*LARGO_INICIAL-1)) {
        pila_redimensionar(pila, false);
    }
    void *dato = pila_ver_tope(pila);
    pila->cantidad--;
    return dato;
}
