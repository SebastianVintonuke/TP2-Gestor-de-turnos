#include "heap.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define LARGO_INICIAL 10

struct heap {
    void **datos;
    size_t cantidad;   // Cantidad de elementos almacenados.
    size_t capacidad;  // Capacidad del arreglo 'datos'.
    cmp_func_t cmp;
};

heap_t *heap_crear(cmp_func_t cmp) {
    heap_t *heap = malloc(sizeof(heap_t));
    if (heap == NULL) {
        return NULL;
    }
    heap->capacidad = LARGO_INICIAL;
    heap->datos = malloc(heap->capacidad * sizeof(void *));
    if (heap->datos == NULL) {
        free(heap);
        return NULL;
    }
    heap->cmp = cmp;
    heap->cantidad = 0;
    return heap;
}

void up_heap(heap_t *heap, size_t pos) {
    if (pos == 0) {
        return;
    }
    size_t padre = (pos - 1) / 2;
    if (heap->cmp(heap->datos[pos], heap->datos[padre]) > 0) {
        void* aux = heap->datos[padre];
        heap->datos[padre] = heap->datos[pos];
        heap->datos[pos] = aux;
        up_heap(heap, padre);
    }
}

void down_heap(void *elementos[], size_t pos, size_t cant, cmp_func_t cmp) {
    size_t h_izq = 2 * pos + 1;
    size_t h_der = 2 * pos + 2;
    size_t h_mayor;
    if (h_izq >= cant) {
        return;
    } else if (h_der >= cant) {
        h_mayor = h_izq;
    } else {
        h_mayor = (cmp(elementos[h_izq], elementos[h_der]) >= 0) ? h_izq : h_der;
    }
    if (cmp(elementos[h_mayor], elementos[pos]) > 0) {
        void* aux = elementos[pos];
        elementos[pos] = elementos[h_mayor];
        elementos[h_mayor] = aux;
        down_heap(elementos, h_mayor, cant, cmp);
    }
}

bool heap_redimensionar(heap_t *heap, bool crecimiento) {
    if (crecimiento) {
        void **datos_nuevo = realloc(heap->datos, (heap->capacidad * 2) * sizeof(void *));
        if (datos_nuevo == NULL) {
            return false;
        }
        heap->datos = datos_nuevo;
        heap->capacidad = heap->capacidad * 2;
    }
    if (!crecimiento) {
        void **datos_nuevo = realloc(heap->datos, (heap->capacidad / 2) * sizeof(void *));
        if (datos_nuevo == NULL) {
            return false;
        }
        heap->datos = datos_nuevo;
        heap->capacidad = heap->capacidad / 2;
    }
    return true;
}

heap_t *heap_crear_arr(void *arreglo[], size_t n, cmp_func_t cmp) {
    heap_t *heap = malloc(sizeof(heap_t));
    if (heap == NULL) {
        return NULL;
    }
    heap->capacidad = n;
    heap->datos = malloc(heap->capacidad * sizeof(void *));
    if (heap->datos == NULL) {
        free(heap);
        return NULL;
    }
    heap->cmp = cmp;
    heap->cantidad = n;
    for (size_t pos = 0; pos < heap->capacidad; pos++) {
        heap->datos[pos] = arreglo[pos];
    }
    for (size_t pos = (heap->cantidad-1); (pos+1) > 0; pos--) {
        down_heap(heap->datos, pos, heap->cantidad, heap->cmp);
    }
    return heap;
}

void heap_destruir(heap_t *heap, void (*destruir_elemento)(void *e)) {
    if (destruir_elemento) {
        for (size_t i = 0; i < heap->cantidad; i++) {
            destruir_elemento(heap->datos[i]);
        }
    }
    free(heap->datos);
    free(heap);
}

size_t heap_cantidad(const heap_t *heap) {
    return heap->cantidad;
}

bool heap_esta_vacio(const heap_t *heap) {
    return heap->cantidad == 0;
}

bool heap_encolar(heap_t *heap, void *elem) {
    if (heap->cantidad == heap->capacidad) {
        if (!(heap_redimensionar(heap, true))) {
            return false;
        }
    }
    heap->datos[heap->cantidad] = elem;
    heap->cantidad++;
    up_heap(heap, heap->cantidad - 1);
    return true;
}

void *heap_ver_max(const heap_t *heap) {
    if (heap_esta_vacio(heap)) {
        return NULL;
    }
    return (heap->datos[0]);
}

void *heap_desencolar(heap_t *heap) {
    if (heap_esta_vacio(heap)) {
        return NULL;
    }
    if ((heap->cantidad * 4 <= heap->capacidad) && (heap->capacidad > 2 * LARGO_INICIAL - 1)) {
        heap_redimensionar(heap, false);
    }
    void *elem = heap->datos[0];
    heap->datos[0] = heap->datos[heap->cantidad - 1];
    heap->cantidad--;
    down_heap(heap->datos, 0, heap->cantidad, heap->cmp);
    return elem;
}

void heap_sort(void *elementos[], size_t cant, cmp_func_t cmp) {
    for (size_t pos = (cant-1); (pos+1) > 0; pos--) {
        down_heap(elementos, pos, cant, cmp);
    }
    while (cant > 0) {
        void* max = elementos[0];
        elementos[0] = elementos[cant - 1]; 
        elementos[cant - 1] = max;
        cant--;
        down_heap(elementos, 0, cant, cmp);
    }
}
