#include "graph.h"

#include <stdlib.h>


const struct object_vtable gvertex_vtable = {
    (void (*) (void *)) gvertex_delete,
    (void * (*) (const void *)) gvertex_copy,
    (int (*) (const void *, const void *)) gvertex_cmp
};


struct gvertex * gvertex_create_ (uint64_t identifier, void * data) {
    struct gvertex * gvertex = malloc(sizeof(struct gvertex));
    object_init(&(gvertex->oh), &gvertex_vtable);
    gvertex->identifier = identifier;
    gvertex->data = data;
    gvertex->edges = list_create();
    return gvertex;
}


struct gvertex * gvertex_create (uint64_t identifier, const void * data) {
    return gvertex_create_(identifier, OCOPY(data));
}


void gvertex_delete (struct gvertex * gvertex) {
    if (gvertex->data != NULL)
        ODEL(gvertex->data);
    ODEL(gvertex->edges);
    free(gvertex);
}


struct gvertex * gvertex_copy (const struct gvertex * gvertex) {
    struct gvertex * copy = gvertex_create(gvertex->identifier,
                                           gvertex->data);
    return copy;
}


int gvertex_cmp (const struct gvertex * lhs, const struct gvertex * rhs) {
    if (lhs->identifier < rhs->identifier)
        return -1;
    else if (lhs->identifier > rhs->identifier)
        return 1;
    return 0;
}


struct list * gvertex_edges (struct gvertex * gvertex) {
    return OCOPY(gvertex->edges);
}


struct list * gvertex_successors (struct gvertex * gvertex) {
    struct list * successors = list_create();
    struct list_it * it;
    for (it = list_it(gvertex->edges);
         it != NULL;
         it = list_it_next(it)) {
        struct gedge * gedge = list_it_data(it);
        if (gedge->head_identifier == gvertex->identifier)
            list_append(successors, gedge);
    }
    return successors;
}


struct list * gvertex_predecessors (struct gvertex * gvertex) {
    struct list * predecessors = list_create();
    struct list_it * it;
    for (it = list_it(gvertex->edges);
         it != NULL;
         it = list_it_next(it)) {
        struct gedge * gedge = list_it_data(it);
        if (gedge->tail_identifier == gvertex->identifier)
            list_append(predecessors, gedge);
    }
    return predecessors;
}




const struct object_vtable gedge_vtable = {
    (void (*) (void *)) gedge_delete,
    (void * (*) (const void *)) gedge_copy,
    NULL
};


struct gedge * gedge_create_ (void * data,
                              uint64_t head_identifier,
                              uint64_t tail_identifier) {
    struct gedge * gedge = malloc(sizeof(struct gedge));
    object_init(&(gedge->oh), &gedge_vtable);
    gedge->data = data;
    gedge->head_identifier = head_identifier;
    gedge->tail_identifier = tail_identifier;
    return gedge;
}


struct gedge * gedge_create (void * data,
                             uint64_t head_identifier,
                             uint64_t tail_identifier) {
    return gedge_create_(OCOPY(data), head_identifier, tail_identifier);
}


void gedge_delete (struct gedge * gedge) {
    if (gedge->data != NULL)
        ODEL(gedge->data);
    free(gedge);
}


struct gedge * gedge_copy (const struct gedge * gedge) {
    return gedge_create(gedge->data,
                        gedge->head_identifier,
                        gedge->tail_identifier);
}




const struct object_vtable graph_vtable = {
    (void (*) (void *)) graph_delete,
    (void * (*) (const void *)) graph_copy,
    NULL
};


struct graph * graph_create () {
    struct graph * graph = malloc(sizeof(struct graph));
    object_init(&(graph->oh), &graph_vtable);
    graph->vertices = tree_create();
    return graph;
}


void graph_delete (struct graph * graph) {
    ODEL(graph->vertices);
    free(graph);
}


struct graph * graph_copy (const struct graph * graph) {
    /* create graph copy */
    struct graph * copy = graph_create();

    /* copy vertices */
    ODEL(copy->vertices);
    copy->vertices = OCOPY(graph->vertices);

    /* go through each vertex and copy over edges */
    struct tree_it * it;
    for (it = tree_it(graph->vertices); it != NULL; it = tree_it_next(it)) {
        struct gvertex * gvertex = (struct gvertex *) tree_it_data(it);
        struct gvertex * copy_vertex;
        copy_vertex = graph_fetch_vertex(copy, gvertex->identifier);
        ODEL(copy_vertex->edges);
        copy_vertex->edges = OCOPY(gvertex->edges);
    }

    return copy;
}


struct gvertex * graph_fetch_vertex (struct graph * graph,
                                     uint64_t identifier) {
    struct gvertex * needle = gvertex_create_(identifier, NULL);
    struct gvertex * gvertex = tree_fetch(graph->vertices, needle);
    ODEL(needle);
    return gvertex;
}


int graph_insert_vertex_ (struct graph * graph,
                          uint64_t identifier,
                          void * data) {
    struct gvertex * gvertex = gvertex_create_(identifier, data);
    tree_insert(graph->vertices, gvertex);
    return 0;
}


int graph_insert_vertex (struct graph * graph,
                         uint64_t identifier,
                         const void * data) {
    return graph_insert_vertex_(graph, identifier, OCOPY(data));
}


int graph_insert_edge_ (struct graph * graph,
                        void * data,
                        uint64_t head_identifier,
                        uint64_t tail_identifier) {
    struct gvertex * head = graph_fetch_vertex(graph, head_identifier);
    struct gvertex * tail = graph_fetch_vertex(graph, tail_identifier);

    if ((head == NULL) || (tail == NULL))
        return -1;

    struct gedge * gedge;
    gedge = gedge_create_(data, head_identifier, tail_identifier);

    list_append(head->edges, gedge);
    list_append_(tail->edges, gedge);

    return 0;
}


int graph_delete_vertex (struct graph * graph, uint64_t identifier) {
    struct gvertex * needle = gvertex_create_(identifier, NULL);
    int result = tree_remove(graph->vertices, needle);
    ODEL(needle);
    return result;
}


int graph_delete_edge (struct graph * graph,
                       uint64_t head_identifier,
                       uint64_t tail_identifier) {
    struct gvertex * head = graph_fetch_vertex(graph, head_identifier);
    struct gvertex * tail = graph_fetch_vertex(graph, tail_identifier);

    if ((head == NULL) || (tail == NULL))
        return -1;

    int found = 0;

    struct list_it * it;
    for (it = list_it(head->edges); it != NULL; it = list_it_next(it)) {
        struct gedge * gedge = (struct gedge *) list_it_data(it);
        if (    (gedge->head_identifier == head_identifier)
             && (gedge->tail_identifier == tail_identifier)) {
            it = list_it_remove(head->edges, it);
            found = 1;
            break;
         }
    }

    if (found == 0)
        return -1;

    found = 0;

    for (it = list_it(tail->edges); it != NULL; it = list_it_next(it)) {
        struct gedge * gedge = (struct gedge *) list_it_data(it);
        if (    (gedge->head_identifier == head_identifier)
             && (gedge->tail_identifier == tail_identifier)) {
            it = list_it_remove(tail->edges, it);
            break;
         }
    }

    if (found == 0)
        return -1;
    return 0;
}
