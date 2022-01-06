// gcc -g -std=c99 -Wall -Wno-unused-function -Wconversion -Wno-sign-conversion -Werror -o zyxcba *.c
// valgrind --leak-check=full --track-origins=yes --show-reachable=yes ./zyxcba doctores.txt pacientes.txt
// ./pruebas.sh ../zyxcba

#include "hash.h"
#include "cola.h"
#include "heap.h"
#include "abb.h"
#include "csv.h"

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include "funciones_tp2.h"
#include "strutil.h"
#include "mensajes.h"

#define COMANDO_PEDIR_TURNO "PEDIR_TURNO"
#define COMANDO_ATENDER "ATENDER_SIGUIENTE"
#define COMANDO_INFORME "INFORME"

/* Funcion de comparacion para el heap, devuelve la mayor antiguedad entre dos pacientes */
int compara_inscripcion(const void *a, const void *b) {
    const paciente_t *paciente_a = a;
    const paciente_t *paciente_b = b;
    const size_t x = paciente_a->inscripcion;
    const size_t y = paciente_b->inscripcion;
    if (x == y) {
        return 0;
    } else {
        return x < y ? 1 : -1;
    }
}

/* Recibe un registro y un doctor y actualiza el numero de pacientes atendidos por dicho doctor */
void actualizar_registro(abb_t *registro, char* doctor) {
    size_t *n_atendidos = (size_t*)abb_obtener(registro, doctor);
    *n_atendidos += 1;
}

/* Valida los comandos ingresados y (de ser valido) ejecuta su correspondiente funcion */
void procesar_comando(hash_t *doctores, hash_t *pacientes, hash_t *especialidades, abb_t *registro, const char* comando, char** parametros) {
	if (strcmp(comando, COMANDO_PEDIR_TURNO) == 0) {
        pedir_turno(pacientes, especialidades, parametros[0], parametros[1], parametros[2]);
	} else if (strcmp(comando, COMANDO_ATENDER) == 0) {
        if (atender_siguiente(doctores, especialidades, parametros[0])) {
            actualizar_registro(registro, parametros[0]);
        }
	} else if (strcmp(comando, COMANDO_INFORME) == 0) {
        informe(registro, doctores, parametros[0], parametros[1]);
	} else {
        printf(ENOENT_CMD, comando);
	}
}

void eliminar_fin_linea(char* linea) {
	size_t len = strlen(linea);
	if (linea[len - 1] == '\n') {
		linea[len - 1] = '\0';
	}
}

/* Procesa la entrada del teclado en formato: COMANDO:PARAMETRO1,PARAMETRO2,PARAMETRO3 */
void procesar_entrada(hash_t *doctores, hash_t *pacientes, hash_t *especialidades, abb_t *registro) {
	char* linea = NULL;
	size_t c = 0;
	while (getline(&linea, &c, stdin) > 0) {
		eliminar_fin_linea(linea);
		char** campos = split(linea, ':');
		if (campos[1] == NULL) {
			printf(ENOENT_FORMATO, linea);
			free_strv(campos);
			continue;	
		}
		char** parametros = split(campos[1], ',');
		if ((strcmp(campos[0],"PEDIR_TURNO") == 0) && ((!parametros[0]) || (!parametros[1]) || (!parametros[2]))) {
	        printf(ENOENT_PARAMS, campos[0]);
	    } else if ((strcmp(campos[0],"ATENDER_SIGUIENTE") == 0) && (!parametros[0])) {
	        printf(ENOENT_PARAMS, campos[0]);
	    } else if ((strcmp(campos[0],"INFORME") == 0) && ((!parametros[0]) || (!parametros[1]))) {
	        printf(ENOENT_PARAMS, campos[0]);
	    } else {
		    procesar_comando(doctores, pacientes, especialidades, registro, campos[0], parametros);
		}
		free_strv(parametros);
		free_strv(campos);
	}
	free(linea);
}

/* Recibe el diccionario de pacientes y parsea el año de inscripcion */
void set_inscripcion(hash_t *pacientes) {
    hash_iter_t *iterador = hash_iter_crear(pacientes);
    while (!hash_iter_al_final(iterador)) {
        size_t *inscripcion = malloc(sizeof(size_t));
        *inscripcion = atoi(hash_obtener(pacientes, hash_iter_ver_actual(iterador)));
        hash_guardar(pacientes, hash_iter_ver_actual(iterador), inscripcion);
        hash_iter_avanzar(iterador);
    }
    hash_iter_destruir(iterador);
}

/* Funcion de destruccion para el struct paciente_t */
void destruir_paciente(void* dato) {
    paciente_t *paciente = (paciente_t*)dato;
        free(paciente->nombre);
        free(paciente);
}

/* Funcion de destruccion para el struct colas_prioridad_t */
void destruir_cola_prioridad(void* dato) {
    colas_prioridad_t *cola_prioridad = (colas_prioridad_t*)dato;
    cola_destruir(cola_prioridad->prioridad_urgente, free);
    heap_destruir(cola_prioridad->prioridad_regular, destruir_paciente);
    free(dato);
}

/* Inicializa el hash especialidades
   Para cada especialidad(clave) inicializa un struct colas_prioridad_t que contiene
   una cola URGENTE, un heap REGULAR y la cantidad total de pacientes en cola */
hash_t *crear_especialidades(hash_t *doctores) {
    hash_t* especialidades = hash_crear(destruir_cola_prioridad);
	if (!especialidades) {
		return NULL;
	}
	hash_iter_t *iterador = hash_iter_crear(doctores);
	char *especialidad;
	while (!hash_iter_al_final(iterador)) {
	    especialidad = hash_obtener(doctores, hash_iter_ver_actual(iterador));
	    if (!hash_pertenece(especialidades, especialidad)) {
	        colas_prioridad_t* colas_prioridad = malloc(sizeof(colas_prioridad_t));
	        colas_prioridad->prioridad_urgente = cola_crear();
	        colas_prioridad->prioridad_regular = heap_crear(compara_inscripcion);
	        colas_prioridad->cantidad = 0;
	        hash_guardar(especialidades, especialidad, colas_prioridad);
	    }
	    hash_iter_avanzar(iterador);
	}
	hash_iter_destruir(iterador);
	return especialidades;
}

/* Inicializa el abb registro
   Contiene el nombre de los doctores y el numero de pacientes atendidos */
abb_t *crear_registro(hash_t *doctores) {
    abb_t *registro = abb_crear(strcmp, free);
    if (!registro) {
        return NULL;
    }
    hash_iter_t *iterador = hash_iter_crear(doctores);
	while (!hash_iter_al_final(iterador)) {
	    if (!abb_pertenece(registro, hash_iter_ver_actual(iterador))) {
	        size_t *i = malloc(sizeof(size_t));
	        *i = 0;
            abb_guardar(registro, hash_iter_ver_actual(iterador), i);
	    }
	    hash_iter_avanzar(iterador);
	}
	hash_iter_destruir(iterador);
    return registro;
}

/* Funcion Main del programa */
int main(int argc, char** argv) {
    if (!argv[1] || !argv[2]) {
        printf(ENOENT_CANT_PARAMS);
        return -1;
    }
    hash_t *doctores = csv_crear_estructura(argv[1]);
    if (!doctores) {
        printf(ENOENT_ARCHIVO, argv[1]);
        return -1;
    }
    hash_t *pacientes = csv_crear_estructura(argv[2]);
    set_inscripcion(pacientes);
    if (!pacientes) {
        printf(ENOENT_ARCHIVO, argv[2]);
        return -1;
    }
    hash_t *especialidades = crear_especialidades(doctores);
    abb_t *registro = crear_registro(doctores);
    procesar_entrada(doctores, pacientes, especialidades, registro);
    hash_destruir(doctores);
    hash_destruir(pacientes);
    hash_destruir(especialidades);
    abb_destruir(registro);
    return 0;
}
