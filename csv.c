#define _POSIX_C_SOURCE 200809L //getline
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"
#define SEPARADOR ','

static void eliminar_fin_linea(char* linea) {
	size_t len = strlen(linea);
	if (linea[len - 1] == '\n') {
		linea[len - 1] = '\0';
	}
}

/* Recibe una ruta de archivo csv, carga los datos en un diccionario y lo devuelve*/
hash_t* csv_crear_estructura(const char* ruta_csv) {
	FILE* archivo = fopen(ruta_csv, "r");
	if (!archivo) {
		return NULL;
	}
	hash_t* hash = hash_crear(free);
	if (!hash) {
		fclose(archivo);
		return NULL;
	}
	char* linea = NULL;
	size_t c = 0;
	while (getline(&linea, &c, archivo) > 0) {
		eliminar_fin_linea(linea);
		char** campos = split(linea, SEPARADOR);
		hash_guardar(hash, strcat(campos[0],"\0"), strdup(campos[1]));
		free_strv(campos);
	}
	free(linea);
	fclose(archivo);
	return hash;
}

