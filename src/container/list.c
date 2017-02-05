#include "list.h"

#include <stdlib.h>


const struct object_vtable list_vtable = {
    (void (*) (void *)) list_delete,
    (void * (*) (const void *)) list_copy,
    NULL
};


struct list * list_create () {
    struct list * list = malloc(sizeof(struct list));

    object_init(&(list->oh), &list_vtable);
    list->front = NULL;
    list->back = NULL;

    return list;
}


void list_delete (struct list * list) {
    struct list_it * it = list->front;
    struct list_it * next;

    while (it != NULL) {
        next = it->next;
        ODEL(it->obj);
        free(it);
        it = next;
    }

    free(list);
}


struct list * list_copy (const struct list * list) {
    struct list * copy = list_create();

    struct list_it * it;
    for (it = list->front; it != NULL; it = it->next) {
        list_append(copy, it->obj);
    }

    return copy;
}


void list_append (struct list * list, const void * obj) {
    list_append_(list, OCOPY(obj));
}


void list_append_ (struct list * list, void * obj) {
    struct list_it * it = malloc(sizeof(struct list_it));
    it->obj = obj;
    it->next = NULL;
    it->prev = NULL;

    if (list->front == NULL) {
        list->front = it;
        list->back = it;
    }
    else {
        list->back->next = it;
        it->prev = list->back;
        list->back = it;
    }
}


void list_append_list (struct list * dst, const struct list * src) {
    struct list * s = (struct list *) src;
    struct list_it * it;
    for (it = list_it(s); it != NULL; it = list_it_next(it)) {
        list_append(dst, list_it_data(it));
    }
}


void list_prepend (struct list * list, const void * obj) {
    list_prepend_(list, OCOPY(obj));
}


void list_prepend_ (struct list * list, void * obj) {
    struct list_it * it = malloc(sizeof(struct list_it));
    it->obj = obj;
    it->prev = NULL;
    it->next = list->front;
    list->front->prev = it;
    list->front = it;
}


void * list_front (struct list * list) {
    if (list->front == NULL)
        return NULL;
    return list->front->obj;
}


void * list_back (struct list * list) {
    if (list->back == NULL)
        return NULL;
    return list->back->obj;
}


void list_pop_front (struct list * list) {
    if (list->front == NULL)
        return;

    struct list_it * tmp = list->front;
    list->front = tmp->next;
    if (list->front != NULL)
        list->front->prev = NULL;

    ODEL(tmp->obj);
    free(tmp);
}


void list_pop_back (struct list * list) {
    if (list->back == NULL)
        return;

    struct list_it * tmp = list->back;
    list->back = list->back->prev;

    if (list->back != NULL)
        list->back->next = NULL;

    ODEL(tmp->obj);
    free(tmp);
}


struct list_it * list_it (struct list * list) {
    return list->front;
}


void * list_it_data (struct list_it * it) {
    return it->obj;
}


struct list_it * list_it_next (struct list_it * it) {
    if (it == NULL)
        return NULL;
    return it->next;
}


struct list_it * list_it_remove (struct list * list, struct list_it * it) {
    if (it == NULL)
        return NULL;

    struct list_it * next = it->next;

    if (it->prev != NULL)
        it->prev->next = it->next;
    if (it->next != NULL)
        it->next->prev = it->prev;

    if (list->front == it)
        list->front = it->next;
    if (list->back == it)
        list->back = it->prev;

    ODEL(it->obj);
    free(it);

    return next;
}


int list_it_append_ (struct list * list, struct list_it * it, void * data) {
    struct list_it * new_it = malloc(sizeof(struct list_it));
    new_it->obj = data;

    /* new it's pointers */
    new_it->prev = it;
    new_it->next = it->next;

    /* next it's pointers */
    if (it->next != NULL)
        it->next->prev = new_it;

    /* prev it's pointers */
    it->next = new_it;

    /* list's pointers */
    if (list->back == it)
        list->back = new_it;

    return 0;
}


int list_it_append (struct list * list, struct list_it * it, const void * data) {
    return list_it_append_(list, it, OCOPY(data));
}


int list_it_prepend_ (struct list * list, struct list_it * it, void * data) {
    struct list_it * new_it = malloc(sizeof(struct list_it));
    new_it->obj = data;

    new_it->prev = it->prev;
    new_it->next = it;

    it->prev = new_it;

    if (new_it->prev != NULL)
        new_it->prev->next = new_it;

    if (list->front == it)
        list->front = new_it;

    return 0;
}

int list_it_prepend (struct list * list, struct list_it * it, const void * data) {
    return list_it_prepend_(list, it, OCOPY(data));
}
