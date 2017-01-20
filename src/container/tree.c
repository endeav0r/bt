#include "tree.h"

#include <stdio.h>
#include <stdlib.h>

const struct object_vtable tree_vtable = {
    (void (*) (void *)) tree_delete,
    (void * (*) (const void *)) tree_copy,
    NULL
};


struct tree * tree_create () {
    struct tree * tree = malloc(sizeof(struct tree));

    object_init(&(tree->oh), &tree_vtable);
    tree->nodes = NULL;

    return tree;
}


void tree_delete_node (struct tree_node * node) {
    if (node == NULL)
        return;

    tree_delete_node(node->left);
    tree_delete_node(node->right);

    ODEL(node->obj);
    free(node);
}


void tree_delete (struct tree * tree) {
    tree_delete_node(tree->nodes);
    free(tree);
}


void tree_copy_node (struct tree * tree, const struct tree_node * node) {
    if (node == NULL)
        return;
    tree_insert(tree, node->obj);
    if (node->left != NULL)
        tree_copy_node(tree, node->left);
    if (node->right != NULL)
        tree_copy_node(tree, node->right);

}


struct tree * tree_copy (const struct tree * tree) {
    struct tree * copy = tree_create();
    tree_copy_node(copy, tree->nodes);
    return copy;
}


int tree_insert (struct tree * tree, const void * obj) {
    return tree_insert_(tree, OCOPY(obj));
}


int tree_insert_ (struct tree * tree, void * obj) {
    int error = 0;
    tree->nodes = tree_node_insert(tree->nodes, tree_node_create(obj), &error);
    return error;
}


void * tree_fetch (struct tree * tree, const void * needle) {
    return tree_node_fetch(tree->nodes, needle);
}


int tree_remove (struct tree * tree, const void * needle) {
    int error = 0;
    tree->nodes = tree_node_delete(tree->nodes, needle, &error);
    return error;
}


struct tree_node * tree_node_create (void * obj) {
    struct tree_node * node = malloc(sizeof(struct tree_node));

    node->obj = obj;
    node->level = 0;
    node->left = NULL;
    node->right = NULL;

    return node;
}


struct tree_node * tree_node_insert (struct tree_node * node,
                                     struct tree_node * new_node,
                                     int * error) {
    if (node == NULL)
        return new_node;
    else if (OCMP(new_node->obj, node->obj) < 0)
        node->left = tree_node_insert(node->left, new_node, error);
    else if (OCMP(new_node->obj, node->obj) > 0)
        node->right = tree_node_insert(node->right, new_node, error);
    else {
        *error = -1;
        return node;
    }

    node = tree_node_skew(node);
    node = tree_node_split(node);

    return node;
}


void * tree_node_fetch (struct tree_node * node, const void * needle) {
    if (node == NULL)
        return NULL;
    else if (OCMP(needle, node->obj) < 0)
        return tree_node_fetch(node->left, needle);
    else if (OCMP(needle, node->obj) > 0)
        return tree_node_fetch(node->right, needle);
    else
        return node->obj;
}


struct tree_node * tree_node_predecessor (struct tree_node * node) {
    if (node->left == NULL)
        return node;
    node = node->left;
    while (node->right != NULL)
        node = node->right;
    return node;
}


struct tree_node * tree_node_successor (struct tree_node * node) {
    if (node->right == NULL)
        return node;
    node = node->right;
    while (node->left != NULL)
        node = node->left;
    return node;
}


struct tree_node * tree_node_decrease_level (struct tree_node * node) {
    if (node == NULL)
        return NULL;

    if ((node->left == NULL) || (node->right == NULL))
        return node;

    int should_be = node->left->level;
    if (should_be > node->right->level)
        should_be = node->right->level;
    should_be += 1;

    if (should_be < node->level) {
        node->level = should_be;
        if (should_be < node->right->level)
            node->right->level = should_be;
    }
    return node;
}


struct tree_node * tree_node_delete (struct tree_node * node,
                                     const void * needle,
                                     int * error) {
    if (node == NULL) {
        *error = -1;
        return NULL;
    }
    else if (OCMP(needle, node->obj) < 0) {
        node->left = tree_node_delete(node->left, needle, error);
    }
    else if (OCMP(needle, node->obj) > 0) {
        node->right = tree_node_delete(node->right, needle, error);
    }
    else {
        if ((node->left == NULL) && (node->right == NULL)) {
            ODEL(node->obj);
            free(node);
            return NULL;
        }
        else if (node->left == NULL) {
            struct tree_node * tmp = tree_node_successor(node);
            ODEL(node->obj);
            node->obj = OCOPY(tmp->obj);
            node->right = tree_node_delete(node->right, tmp->obj, error);
        }
        else if (node->right == NULL) {
            struct tree_node * tmp = tree_node_predecessor(node);
            ODEL(node->obj);
            node->obj = OCOPY(tmp->obj);
            node->left = tree_node_delete(node->left, tmp->obj, error);
        }
    }

    node = tree_node_decrease_level(node);
    node = tree_node_skew(node);
    node->right = tree_node_skew(node->right);

    if (node->right != NULL)
        node->right->right = tree_node_skew(node->right->right);

    node = tree_node_split(node);
    node->right = tree_node_split(node->right);

    return node;
}


struct tree_node * tree_node_skew (struct tree_node * node) {
    if (node == NULL)
        return NULL;
    else if (node->left == NULL)
        return node;
    else if (node->left->level == node->level) {
        struct tree_node * tmp = node->left;
        node->left = tmp->right;
        tmp->right = node;
        return tmp;
    }
    return node;
}


struct tree_node * tree_node_split (struct tree_node * node) {
    if (node == NULL)
        return NULL;
    else if ((node->right == NULL) || (node->right->right == NULL))
        return node;
    else if (node->level == node->right->right->level) {
        struct tree_node * tmp = node->right;
        node->right = tmp->left;
        tmp->left = node;
        tmp->level++;
        return tmp;
    }
    return node;
}


const struct object_vtable tree_it_obj_vtable = {
    (void (*) (void *)) tree_it_obj_delete,
    (void * (*) (const void *)) tree_it_obj_copy,
    NULL
};


struct tree_it_obj * tree_it_obj_create (struct tree_node * node) {
    struct tree_it_obj * tio = malloc(sizeof(struct tree_it_obj));
    object_init(&(tio->oh), &tree_it_obj_vtable);
    tio->node = node;
    return tio;
}


void tree_it_obj_delete (struct tree_it_obj * tio) {
    free(tio);
}


struct tree_it_obj * tree_it_obj_copy (const struct tree_it_obj * tio) {
    return tree_it_obj_create(tio->node);
}


enum {
    LEFT,
    RIGHT
};


int tree_it_walk_left (struct list * list);


struct tree_it * tree_it (struct tree * tree) {
    if (tree->nodes == NULL)
        return NULL;

    struct tree_it * it = malloc(sizeof(struct tree_it));
    it->list = list_create();
    list_append_(it->list, tree_it_obj_create(tree->nodes));

    tree_it_walk_left(it->list);

    return it;
}


void tree_it_delete (struct tree_it * it) {
    ODEL(it->list);
    free(it);
}


void * tree_it_data (struct tree_it * it) {
    struct tree_it_obj * tio = list_back(it->list);
    return tio->node;
}


int tree_it_walk_left (struct list * list) {
    struct tree_it_obj * tio = list_back(list);
    while (tio->node->left != NULL) {
        list_append_(list, tree_it_obj_create(tio->node->left));
        tio = list_back(list);
    }
    return 0;
}


struct tree_it * tree_it_next (struct tree_it * it) {
    struct tree_it_obj * tio = list_back(it->list);
    if (tio->node->right) {
        struct tree_node * tmp = tio->node->right;
        list_pop_back(it->list);
        list_append_(it->list, tree_it_obj_create(tmp));
        tree_it_walk_left(it->list);
        return it;
    }
    else {
        list_pop_back(it->list);
        if (list_front(it->list) == NULL) {
            tree_it_delete(it);
            return NULL;
        }
        return it;
    }
}


struct tree * tree_map (struct tree * tree, void (* f) (void *)) {
    struct tree * result = tree_create();
    struct tree_it * it;
    for (it = tree_it(tree); it != NULL; it = tree_it_next(it)) {
        void * obj = OCOPY(tree_it_data(it));
        f(obj);
        tree_insert(result, obj);
    }
    return result;
}
