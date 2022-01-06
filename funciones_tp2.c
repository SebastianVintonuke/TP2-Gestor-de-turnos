#include "hash.h"
#include "abb.h"
#include "cola.h"
#include "heap.h"
#include "mensajes.h"
#include "funciones_tp2.h"

void pedir_turno(hash_t *pacientes, hash_t *especialidades, char *paciente, char *especialidad, char *urgencia) {
    if (hash_pertenece(pacientes, paciente)) {
        if (hash_pertenece(especialidades, especialidad)) {
                colas_prioridad_t *cola_prioridad = hash_obtener(especialidades, especialidad); 
            if (strcmp(urgencia,"URGENTE") == 0) {
                cola_t *cola = cola_prioridad->prioridad_urgente; 
                cola_encolar(cola, strdup(paciente));
            } else if (strcmp(urgencia,"REGULAR") == 0) {
                paciente_t *struct_paciente = malloc(sizeof(paciente_t));
                struct_paciente->nombre = strdup(paciente);
                struct_paciente->inscripcion = *(size_t*)hash_obtener(pacientes, paciente);
                heap_t *heap = cola_prioridad->prioridad_regular;
                heap_encolar(heap, struct_paciente);
            } else {
                printf(ENOENT_URGENCIA, urgencia);
                return;
            }
            cola_prioridad->cantidad += 1;    
        } else {
            printf(ENOENT_ESPECIALIDAD, especialidad);
            return;
        }
    } else {
        printf(ENOENT_PACIENTE, paciente);
        return;
    }
    colas_prioridad_t *cola_prioridad = hash_obtener(especialidades, especialidad);
    size_t n_pacientes = cola_prioridad->cantidad;
    printf(PACIENTE_ENCOLADO, paciente);
    printf(CANT_PACIENTES_ENCOLADOS, n_pacientes, especialidad);
}

bool atender_siguiente(hash_t *doctores, hash_t *especialidades, char *doctor) {
    char *especialidad;
    if (hash_pertenece(doctores, doctor)) {
        especialidad = hash_obtener(doctores, doctor);
    } else  {
        printf(ENOENT_DOCTOR, doctor);
        return false;
    }
    colas_prioridad_t *colas_prioridad = hash_obtener(especialidades, especialidad);
    char *paciente;
    if (!cola_esta_vacia(colas_prioridad->prioridad_urgente)) {
        paciente = cola_desencolar(colas_prioridad->prioridad_urgente);     
    } else if (!heap_esta_vacio(colas_prioridad->prioridad_regular)) {
        paciente_t *struct_paciente = (paciente_t*)heap_desencolar(colas_prioridad->prioridad_regular);
        paciente =  struct_paciente->nombre;
        free(struct_paciente);
    } else {
        printf(SIN_PACIENTES);
        return false;
    }
    colas_prioridad->cantidad -= 1;
    size_t n_pacientes = colas_prioridad->cantidad;
    printf(PACIENTE_ATENDIDO, paciente);
    printf(CANT_PACIENTES_ENCOLADOS, n_pacientes, especialidad);
    free(paciente);
    return true;
}

char *agregar_actual(abb_t *registro, hash_t *doctores, abb_iter_t *iterador, size_t i) {
    char *linea = malloc(140*sizeof(char));
    const char* doctor = abb_iter_in_ver_actual(iterador);
    char* especialidad = hash_obtener(doctores, doctor);
    size_t atendidos = *(size_t*)abb_obtener(registro, doctor);
    sprintf(linea, INFORME_DOCTOR, i, doctor, especialidad, atendidos);
    return linea;
}

void informe(abb_t *registro, hash_t *doctores, char* inicio, char* fin) {
    size_t i = 0;
    size_t cantidad = 0;
    cola_t *cola_informe = cola_crear();
    
    /* SI INICIO Y FIN NO SON DOCTORES LOS AGREGO */
    if (strcmp(fin, "") != 0) {
        if (!hash_pertenece(doctores, fin)) {
            abb_guardar(registro, fin, malloc(sizeof(size_t)));
        }
    }
    if (strcmp(inicio, "") != 0) {
        if (!hash_pertenece(doctores, inicio)) {
            abb_guardar(registro, inicio, malloc(sizeof(size_t)));
        }
    }
    
    /* CREO EL ITERADOR */
    abb_iter_t *iterador = abb_iter_in_crear(registro);
    
    /* Si HAY INICIO, MUEVO EL ITERADOR HASTA INICIO */
    if (strcmp(inicio, "") != 0) {
        while(strcmp(abb_iter_in_ver_actual(iterador), inicio) != 0) {
            abb_iter_in_avanzar(iterador);
        }
        /* SI INICIO NO ES UN DOCTOR, MUEVO EL ITERADOR HASTA INICIO+1 */
        if (!hash_pertenece(doctores, fin)) {
            abb_iter_in_avanzar(iterador);
        }
    }
    
    /* SI HAY FIN, ITERO HASTA EL FIN O HASTA QUE ITERADOR ESTE AL FINAL */
    if (strcmp(fin, "") != 0) {
        while(!abb_iter_in_al_final(iterador) && (strcmp(abb_iter_in_ver_actual(iterador), fin) != 0)) {
            i += 1;
            cantidad += 1;
            cola_encolar(cola_informe, agregar_actual(registro, doctores, iterador, i));
            abb_iter_in_avanzar(iterador);
        }
        /* SI FIN ES UN DOCTOR LO IMPRIMO */
        if (hash_pertenece(doctores, fin)) {
            i += 1;
            cantidad += 1;
            cola_encolar(cola_informe, agregar_actual(registro, doctores, iterador, i));
        }  
    
    /* SI NO HAY FIN, ITERO HASTA QUE ITERADOR ESTE AL FINAL */
    } else {
        while(!abb_iter_in_al_final(iterador)) {
            i += 1;
            cantidad += 1;
            cola_encolar(cola_informe, agregar_actual(registro, doctores, iterador, i));
            abb_iter_in_avanzar(iterador);
        }
    }
    
    /* IMPRIMO EL INFORME */
    printf(DOCTORES_SISTEMA, cantidad);
    while (!cola_esta_vacia(cola_informe)) {
        char *linea = cola_ver_primero(cola_informe);
        printf("%s", linea);
        free(cola_desencolar(cola_informe));
    }
    
    /* DESTRUYO LA COLA */
    cola_destruir(cola_informe, NULL);
    
    /* DESTRUYO EL ITERADOR */
    abb_iter_in_destruir(iterador);
    
    /* SI AGREGUE INICIO O FIN LOS BORRO */
    if (strcmp(inicio, "") != 0) {
        free(abb_borrar(registro, inicio));
    }
    if (strcmp(fin, "") != 0) {
        free(abb_borrar(registro, fin));
    }
}
