#ifndef graph_HEADER
#define graph_HEADER

#include "list.h"
#include "tree.h"

#include <inttypes.h>


struct gvertex {
    const struct object * object;
    uint64_t identifier;
    void * data;
    struct list * edges;
};

struct gvertex * gvertex_create_ (uint64_t identifier, void * data);
struct gvertex * gvertex_create  (uint64_t identifier, const void * data);
void             gvertex_delete  (struct gvertex *);
struct gvertex * gvertex_copy    (const struct gvertex *);
int              gvertex_cmp (const struct gvertex * lhs,
     const struct gvertex * rhs);
struct list * gvertex_edges        (struct gvertex *);
struct list * gvertex_successors   (struct gvertex *);
struct list * gvertex_predecessors (struct gvertex *);


struct gedge {
    const struct object * object;
    void * data;
    uint64_t head_identifier;
    uint64_t tail_identifier;
};

struct gedge * gedge_create_ (void * data,
                              uint64_t head_identifier,
                              uint64_t tail_identifier);
struct gedge * gedge_create (void * data,
                             uint64_t head_identifier,
                             uint64_t tail_identifier);
void gedge_delete (struct gedge * gedge);
struct gedge * gedge_copy (const struct gedge * gedge);


struct graph {
    const struct object * object;
    struct tree * vertices;
};


struct graph * graph_create ();
void           graph_delete (struct graph *);
struct graph * graph_copy (const struct graph *);

struct gvertex * graph_fetch_vertex (struct graph *, uint64_t identifier);
int graph_insert_vertex_ (struct graph *, uint64_t identifier, void * data);
int graph_insert_vertex  (struct graph *,
                          uint64_t identifier,
                          const void * data);
int graph_insert_edge (struct graph *,
                       void * data,
                       uint64_t head_identifier,
                       uint64_t tail_identifier);
int graph_delete_vertex (struct graph *, uint64_t identifier);
int graph_delete_edge   (struct graph *,
                         uint64_t head_identifier,
                         uint64_t tail_identifier);

#endif
