#ifndef tree_HEADER
#define tree_HEADER

#include "list.h"
#include "object.h"

struct tree_node {
    void * obj;
    int level;
    struct tree_node * left;
    struct tree_node * right;
};


struct tree {
    struct object_header oh;
    struct tree_node * nodes;
};

struct tree * tree_create ();
void          tree_delete (struct tree * tree);
struct tree * tree_copy   (const struct tree * tree);

/**
* Inserts an object into the tree. This form of the function creates a copy of
* the passed object. If a duplicate object already exists, the tree remains
* unchanged and -1 is returned.
*
* @param tree The tree to insert the object into.
* @param obj The object to insert into the tree.
* @return 0 if the object was successfully inserted, and non-zero if the object
*         was not inserted.
*/
int    tree_insert  (struct tree * tree, const void * obj);

/**
* Inserts an object into the tree. This form of the function takes ownership of
* the passed object. If a duplicate object already exists, the tree remains
* unchanged, the passed object is deleted, and -1 is returned.
*
* @param tree The tree to insert the object into.
* @param obj The object to insert into the tree.
* @return 0 if the object was successfully inserted, and non-zero if the object
*           was not inserted.
*/
int    tree_insert_ (struct tree * tree, void * obj);
void * tree_fetch   (struct tree * tree, const void * needle);
int    tree_remove  (struct tree * tree, const void * needle);

struct tree_node * tree_node_create (void * obj);

struct tree_node * tree_node_insert  (struct tree_node * node,
                                      struct tree_node * new_node,
                                      int * error);
void * tree_node_fetch (struct tree_node * node, const void * needle);
struct tree_node * tree_node_delete  (struct tree_node * node,
                                      const void * needle,
                                      int * error);

struct tree_node * tree_node_skew  (struct tree_node * node);
struct tree_node * tree_node_split (struct tree_node * node);


struct tree_it_obj {
    struct object_header oh;
    struct tree_node * node;
};

struct tree_it_obj * tree_it_obj_create (struct tree_node * node);
void                 tree_it_obj_delete (struct tree_it_obj * tio);
struct tree_it_obj * tree_it_obj_copy   (const struct tree_it_obj * tio);

struct tree_it {
    struct list * list;
    int step;
};


struct tree_it * tree_it        (struct tree * tree);
void             tree_it_delete (struct tree_it * it);
void *           tree_it_data   (struct tree_it * it);
struct tree_it * tree_it_next   (struct tree_it * it);

struct tree * tree_map (struct tree * tree, void (* f) (void *));

#endif
