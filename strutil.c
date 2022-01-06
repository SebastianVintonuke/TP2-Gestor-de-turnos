#include "strutil.h"

#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


char *substr(const char *str, size_t n) {
    size_t size = strlen(str);
    if (n>=size) {
        char *newstr = strdup(str);
        if(!newstr) {
            return NULL;
        }
        return newstr;
    }
    char *newstr = malloc(sizeof(char)*(n+2));
    if(!newstr) {
       return NULL;
    }
    size_t i = 0;
    while (i<n) {
        newstr[i] = str[i];
        i++;
    }
    newstr[i] = '\0';
    return newstr;
}

char **split(const char *str, char sep) {
    if (strcmp(str,"") == 0) {
        char **strv = malloc(sizeof(char*) * 2);
        if (!strv) {
            return NULL;
        }
        strv[0] = substr(str, 0);
        if(!strv[0]) {
            free_strv(strv);
            return NULL;
        }
        strv[1] = NULL;
        return strv;
    }   
    
    int n_sep = 0;
    for (int pos = 0; str[pos] != '\0'; pos++) {
        if (str[pos] == sep) {
            n_sep++;
        }
    }
    char **strv = malloc(sizeof(char*) * (n_sep + 2));
    if (!strv) {
       return NULL;
    }
    int pos_v = 0;
    int pos = 0;
    int i = 0;
    while (str[i] != '\0') {
        i = pos;
        while (str[i] != '\0' && str[i] != sep) {
            i++;
        }
        char *newstr = substr(str+pos, i-pos);
        if(!newstr) {
            free_strv(strv);
            return NULL;
        }
        strv[pos_v++] = newstr;
        pos = i;
        pos++;
    }
    strv[pos_v] = NULL;
    return strv;
}

char *join(char **strv, char sep) {
    char *newstr;
    if (strv[0] == NULL) {
        newstr = malloc(sizeof(char));
        newstr[0] = '\0';
        return newstr;
    }
    int i = 0;
    size_t n_size = 0;
    while (strv[i] != NULL) {
        n_size += strlen(strv[i]);
        i++;
    }
    if (sep != '\0') {
        newstr = malloc(sizeof(char)*(n_size+i));
    } else {
        newstr = malloc(sizeof(char)*(++n_size));
    }
    if(!newstr) {
        return NULL;
    }
    int pos = 0;
    i = 0;
    while (strv[i] != NULL) {
        for (int i2 = 0; strv[i][i2] != '\0'; i2++) {
            newstr[pos++] = strv[i][i2];
        }
        if (sep != '\0') {
            newstr[pos++] = sep;
        }
        i++;
    }
    if (sep != '\0') {
        newstr[--pos] = '\0';
    } else {
        newstr[pos] = '\0';
    }
    return newstr;
}

void free_strv(char *strv[]) {
    for (int i = 0; strv[i] != NULL; i++) {
        free(strv[i]);
    }
    free(strv);
}
