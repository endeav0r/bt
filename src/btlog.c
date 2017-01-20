#include "btlog.h"

#include "container/list.h"

#include "object.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct btlog_object * btlog_object_create (const char * line);
void                  btlog_object_delete (struct btlog_object * btlog_object);
struct btlog_object * btlog_object_copy   (const struct btlog_object * btlog_object);



struct list * btlog_list = NULL;
FILE * btlog_fh = NULL;



const struct object btlog_object_object = {
    (void (*) (void *)) btlog_object_delete,
    (void * (*) (const void *)) btlog_object_copy,
    NULL
};

struct btlog_object {
    const struct object * object;
    char * line;
};


struct btlog_object * btlog_object_create (const char * line) {
    struct btlog_object * btlog_object = malloc(sizeof(struct btlog_object));
    btlog_object->object = &btlog_object_object;
    btlog_object->line = strdup(line);
    return btlog_object;
}


void btlog_object_delete (struct btlog_object * btlog_object) {
    free(btlog_object->line);
    free(btlog_object);
}


struct btlog_object * btlog_object_copy (const struct btlog_object * btlog_object) {
    return btlog_object_create(btlog_object->line);
}


void btlog_continuous (const char * filename) {
    btlog_fh = fopen(filename, "wb");
}


void btlog (const char * format, ...) {
    va_list args;
    va_start(args, format);
    // vasprintf support is a pain
    size_t str_size = 64;
    char * str = malloc(str_size);
    do {
        size_t sprintf_chars = vsnprintf(str, str_size, format, args);
        if (sprintf_chars >= str_size - 1) {
            free(str);
            str_size *= 2;
            str = malloc(str_size);
        }
        else
            break;
    } while(1);
    va_end(args);

    if (btlog_fh != NULL) {
        fprintf(btlog_fh, "%s\n", str);
        fflush(btlog_fh);
    }
    else {
        if (btlog_list == NULL)
            btlog_list = list_create();

        list_append_(btlog_list, btlog_object_create(str));
    }

    free(str);
}


void write_btlog (const char * filename) {
    FILE * fh = fopen(filename, "wb");

    if (fh == NULL)
        return;

    struct list_it * it;

    for (it = list_it(btlog_list); it != NULL; it = list_it_next(it)) {
        struct btlog_object * btlog_object = list_it_data(it);
        fprintf(fh, "%s\n", btlog_object->line);
    }

    fclose(fh);
}
