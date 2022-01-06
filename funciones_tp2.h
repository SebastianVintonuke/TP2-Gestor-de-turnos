#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct colas_prioridad {
    size_t cantidad;
    cola_t *prioridad_urgente;
    heap_t *prioridad_regular;
}colas_prioridad_t;

typedef struct paciente {
    char *nombre;
    size_t inscripcion;
}paciente_t;

/* Se recibe un nombre de paciente y el nombre de una especialidad, y el sistema le
 * añade a la lista de espera de la especialidad correspondiente.
 * Pre: Especialidades fue inicializado.
 * Post: El paciente fue agregado a la cola o al heap de su especialidad
 * correspondiente dependiendo de su urgencia.
 */
void pedir_turno(hash_t *pacientes, hash_t *especialidades, char *paciente, char *especialidad, char *urgencia);

/* Se recibe el nombre de le doctore y este atiende al siguiente paciente urgente
 * (por orden de llegada). Si no hubiera ningún paciente urgente, atiende al
 * siguiente paciente con mayor antiguedad como paciente de la clinica.
 * Devuelve True si se atendio un paciente, false en caso contrario.
 * Pre: Especialidades fue inicializado.
 * Post: El paciente correspondiente fue desencolado.
 */
bool atender_siguiente(hash_t *doctores, hash_t *especialidades, char *doctor);

/* Imprime la lista de doctores en orden alfabético, junto con su especialidad y el
 * número de pacientes que atendieron desde que arranco el sistema.
 * Opcionalmente, se puede especificar el rango (alfabético) de doctores sobre los
 * que se desea informe.
 * Pre: Registro fue inicializado.
 * Post: El registro correspondiente fue impreso.
 */
void informe(abb_t *registro, hash_t *doctores, char* inicio, char* fin);
