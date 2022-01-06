#include "hash.h"

#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define TAM_INICIAL 11
#define FACTOR_DE_CARGA_ARRIBA 70
#define FACTOR_DE_CARGA_ABAJO 15

enum state {
    VACIO,           // ESPACIO VACIO
    OCUPADO,         // ESPACIO OCUPADO
    DESOCUPADO,      // ESPACIO PREVIAMENTE OCUPADO
};

typedef struct dato {
    int estado;
    char *clave;
    void *dato;
}dato_t;

struct hash {
    dato_t** datos;
    size_t cantidad_lleno;      // CANTIDAD DE ESPACIO OCUPADO
    size_t cantidad_vaciado;    // CANTIDAD DE ESPACIO PREVIAMENTE OCUAPADO
    size_t capacidad;
    hash_destruir_dato_t funcion_de_destruccion;
};

struct hash_iter {
    const hash_t *hash;
    size_t pos;
};


//-----------------------------------------------------------------------//

size_t factor_de_carga_actual(const hash_t *hash) {
    size_t ocupados = hash->cantidad_lleno;
    size_t borrados = hash->cantidad_vaciado; // SE TOMAN EN CUENTA LOS BORRADOS
    size_t capacidad = hash->capacidad;
    return ((ocupados + borrados) * 100) / capacidad;
}

// FUNCIÓN DE HASH  //
// https://en.wikipedia.org/wiki/Jenkins_hash_function //
size_t jenkins_one_at_a_time_hash(const char* key, size_t length) {
  size_t i = 0;
  size_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato) {
    hash_t *hash = malloc(sizeof(hash_t));
    if (hash == NULL) {
        return NULL;
    }
    hash->capacidad = TAM_INICIAL;
    hash->datos = malloc(hash->capacidad * sizeof(dato_t*));
    if (hash->datos == NULL) {
        free(hash);
        return NULL;
    }
    size_t pos = 0;
    while (pos < hash->capacidad) {
        dato_t *nuevo_dato = malloc(sizeof(dato_t));
        if (nuevo_dato == NULL) {
            pos--;
            while (pos > 0) {
                free(hash->datos[pos]);
                pos--;
            }
            free(hash);
            return NULL;
        }
        nuevo_dato->estado = VACIO;
        hash->datos[pos] = nuevo_dato;
        pos++;
    }
    hash->funcion_de_destruccion = destruir_dato;
    hash->cantidad_lleno = 0;
    hash->cantidad_vaciado = 0;
    return hash;
}

size_t buscar(const hash_t *hash, const char *clave) {
    size_t clave_hasheada = jenkins_one_at_a_time_hash(clave, strlen(clave));
    size_t pos_inicial = clave_hasheada % hash->capacidad;
    size_t pos = pos_inicial;
    do {
        if (hash->datos[pos]->estado == OCUPADO && strcmp(hash->datos[pos]->clave, clave) == 0) {
            return pos;
        }
        pos++;
        if (pos >= hash->capacidad) {
            pos = 0;
        }
    } while (hash->datos[pos]->estado != VACIO && pos != pos_inicial);
    return -1;
}

// Resimensióna el hash. Recibe la nueva capacidad //
// Devuelve un booleano indicando si se logro la redimensión //
// Pre: La estructura hash fue inicializada //
// Post: Se redimensióna el hash //
bool hash_redimensionar(hash_t *hash, size_t nueva_capacidad) {
    dato_t **datos_aux = hash->datos;
    size_t capacidad_aux = hash->capacidad;
    dato_t **datos_nuevo = malloc(nueva_capacidad * sizeof(dato_t*));
    if (datos_nuevo == NULL) {
            return false;
    }
    size_t pos = 0;
    while (pos < nueva_capacidad) {
        dato_t *nuevo_dato = malloc(sizeof(dato_t));
        if (nuevo_dato == NULL) {
            pos--;
            while (pos > 0) {
                free(hash->datos[pos]);
                pos--;
            }
        return false;
        }
        nuevo_dato->estado = VACIO;
        datos_nuevo[pos] = nuevo_dato;
        pos++;
    }
    hash->datos = datos_nuevo;
    hash->capacidad = nueva_capacidad;
    pos = 0;
    hash->cantidad_vaciado = 0;
    hash->cantidad_lleno = 0;
    while (pos < capacidad_aux) {
        if (datos_aux[pos]->estado == OCUPADO) {
            hash_guardar(hash, datos_aux[pos]->clave, datos_aux[pos]->dato);
            free(datos_aux[pos]->clave);
        }
        free(datos_aux[pos]);
        pos++;
    } 
    free(datos_aux);
    return true;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato) {
    if (hash->capacidad == hash->cantidad_lleno) {
        if (!(hash_redimensionar(hash, hash->capacidad * 2))) {
            return false;
        }
    }
    size_t pos = buscar(hash, clave);
    if (pos != -1) {
        if (hash->funcion_de_destruccion != NULL) {
            hash->funcion_de_destruccion(hash->datos[pos]->dato);
        }
        hash->datos[pos]->dato = dato;
    } else {    
        size_t clave_hasheada = jenkins_one_at_a_time_hash(clave, strlen(clave));
        pos = clave_hasheada % hash->capacidad;
        while (true) {
            if (pos >= hash->capacidad) {
                pos = 0;
            }
            if (hash->datos[pos]->estado == VACIO || hash->datos[pos]->estado == DESOCUPADO) {
                if (hash->datos[pos]->estado == DESOCUPADO) {
                    hash->cantidad_vaciado--;
                }
                hash->datos[pos]->clave = strdup(clave);
                hash->datos[pos]->dato = dato;
                hash->datos[pos]->estado = OCUPADO;
                hash->cantidad_lleno++;
                break;
            }
            pos++;
        }
    }
    // COMPROBACIÓN DE REDIMENSIÓN //
    if (factor_de_carga_actual(hash) > FACTOR_DE_CARGA_ARRIBA) {
        hash_redimensionar(hash, hash->capacidad * 2);
    }
    return true;
}

void *hash_borrar(hash_t *hash, const char *clave) { 
    size_t pos = buscar(hash, clave);
    if (pos == -1) {
        return NULL;
    }
    free(hash->datos[pos]->clave);
    hash->datos[pos]->estado = DESOCUPADO;
    hash->cantidad_lleno--;
    hash->cantidad_vaciado++;
    // COMPROBACIÓN DE REDIMENSIÓN //
    if (factor_de_carga_actual(hash) < FACTOR_DE_CARGA_ABAJO && hash->capacidad > (TAM_INICIAL * 2)) {
        hash_redimensionar(hash, hash->capacidad / 2);
    }
    return hash->datos[pos]->dato;
}

void *hash_obtener(const hash_t *hash, const char *clave) {
    size_t pos = buscar(hash, clave);
    if (pos == -1) {
        return NULL;
    }
    return hash->datos[pos]->dato;
}

bool hash_pertenece(const hash_t *hash, const char *clave) {
    return buscar(hash, clave) != -1;       
}

size_t hash_cantidad(const hash_t *hash) {
    return hash->cantidad_lleno;
}

void hash_destruir(hash_t *hash) {
    size_t pos = 0;
    while (pos < hash->capacidad) {
        if (hash->datos[pos]->estado == OCUPADO) {
            if (hash->funcion_de_destruccion != NULL) {
                hash->funcion_de_destruccion(hash->datos[pos]->dato);
            }
            free(hash->datos[pos]->clave);
        }
        free(hash->datos[pos]);
        pos++;
    }
    free(hash->datos);
    free(hash);
}

//-----------------------------------------------------------------------//

hash_iter_t *hash_iter_crear(const hash_t *hash) {
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if(!iter) {
        return NULL;
    }
    iter->hash = hash;
    iter->pos = 0;
    while (iter->hash->datos[iter->pos]->estado == VACIO || hash->datos[iter->pos]->estado == DESOCUPADO) {
        if (hash->cantidad_lleno == 0) {
            iter->pos = iter->hash->capacidad;
            break;
        }
        iter->pos++;
    }
    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter) {
    if (hash_iter_al_final(iter)) {
        return false;
    }
    iter->pos++;
    while (!(hash_iter_al_final(iter))) {
        if (iter->hash->datos[iter->pos]->estado == OCUPADO) {
            return true;
        } else {
            iter->pos++;
        }
    }
    return false;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter) {
    if (hash_iter_al_final(iter)) {
        return NULL;
    }
    return (iter->hash->datos[iter->pos])->clave;
}

bool hash_iter_al_final(const hash_iter_t *iter) {
    if (iter->hash->capacidad == iter->pos) {
        return true;
    }
    return false;
}

void hash_iter_destruir(hash_iter_t *iter) {
    free(iter);
}
