#ifndef tree_HEADER
#define tree_HEADER

#include "object.h"

struct tree_node {
    void * obj;
    int level;
    struct tree_node * left;
    struct tree_node * right;
};


struct tree {
    const struct object * object;
    struct tree_node * nodes;
};

struct tree * tree_create ();
void          tree_delete (struct tree * tree);
struct tree * tree_copy   (const struct tree * tree);

int    tree_insert  (struct tree * tree, const void * obj);
int    tree_insert_ (struct tree * tree, void * obj);
void * tree_fetch   (struct tree * tree, const void * needle);
int    tree_remove  (struct tree * tree, const void * needle);

struct tree_node * tree_node_create (void * obj);

struct tree_node * tree_node_insert  (struct tree_node * node,
                                      struct tree_node * new_node,
                                      int * error);
void * tree_node_fetch (struct tree_node * node,
                        const struct tree_node * needle);
struct tree_node * tree_node_delete  (struct tree_node * node,
                                      const struct tree_node * needle,
                                      int * error);

struct tree_node * tree_node_skew  (struct tree_node * node);
struct tree_node * tree_node_split (struct tree_node * node);

#endif