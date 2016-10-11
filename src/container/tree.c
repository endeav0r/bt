#include "tree.h"

#include <stdlib.h>

const struct object tree_object = {
    (void (*) (void *)) tree_delete,
    (void * (*) (const void *)) tree_copy,
    NULL
};


struct tree * tree_create () {
    struct tree * tree = malloc(sizeof(struct tree));

    tree->object = &tree_object;
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


int tree_insert (struct tree * tree, const void * obj) {
    return tree_insert_(tree, OCOPY(obj));
}


int tree_insert_ (struct tree * tree, void * obj) {
    int error = 0;
    tree->nodes = tree_node_insert(tree->nodes, obj, &error);
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


void * tree_node_fetch (struct tree_node * node,
                        const struct tree_node * needle) {
    if (node == NULL)
        return NULL;
    else if (OCMP(needle->obj, node->obj) < 0)
        return tree_node_fetch(node->left, needle);
    else if (OCMP(needle->obj, node->obj) > 0)
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
                                     const struct tree_node * needle,
                                     int * error) {
    if (node == NULL) {
        *error = -1;
        return NULL;
    }
    else if (OCMP(needle->obj, node->obj) < 0)
        node->left = tree_node_delete(node->left, needle, error);
    else if (OCMP(needle->obj, node->obj) > 0)
        node->right = tree_node_delete(node->right, needle, error);
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
            node->right = tree_node_delete(node->right, tmp, error);
        }
        else if (node->right == NULL) {
            struct tree_node * tmp = tree_node_predecessor(node);
            ODEL(node->obj);
            node->obj = OCOPY(tmp->obj);
            node->left = tree_node_delete(node->left, tmp, error);
        }
    }

    node = tree_node_decrease_level(node);
    node = tree_node_skew(node);
    node->right = tree_node_skew(node->right);

    if (node->right != NULL)
        node->right = tree_node_skew(node->right->right);

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