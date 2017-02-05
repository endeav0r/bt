#include "varstore.h"

#include <stdlib.h>
#include <string.h>

const struct object_vtable varstore_node_vtable = {
    (void (*) (void *)) varstore_node_delete,
    (void * (*) (const void *)) varstore_node_copy,
    (int (*) (const void *, const void *)) varstore_node_cmp
};


struct varstore_node * varstore_node_create (const char * identifier,
                                             size_t bits,
                                             size_t offset) {
    struct varstore_node * vn = malloc(sizeof(struct varstore_node));
    object_init(&(vn->oh), &varstore_node_vtable);
    vn->identifier = strdup(identifier);
    vn->bits = bits;
    vn->offset = offset;
    return vn;
}


void varstore_node_delete (struct varstore_node * vn) {
    free(vn->identifier);
    free(vn);
}


struct varstore_node * varstore_node_copy (const struct varstore_node * vn) {
    return varstore_node_create(vn->identifier, vn->bits, vn->offset);
}


int varstore_node_cmp (
    const struct varstore_node * lhs,
    const struct varstore_node * rhs
) {
    int strcmp_r = strcmp(lhs->identifier, rhs->identifier);
    if (strcmp_r)
        return strcmp_r;
    else if (lhs->bits < rhs->bits)
        return -1;
    else if (rhs->bits > lhs->bits)
        return 1;
    else
        return 0;
}


const struct object_vtable varstore_vtable = {
    (void (*) (void *)) varstore_delete,
    (void * (*) (const void *)) varstore_copy,
    NULL
};


struct varstore * varstore_create () {
    struct varstore * varstore = malloc(sizeof(struct varstore));
    object_init(&(varstore->oh), &varstore_vtable);
    varstore->tree = tree_create();
    varstore->data_buf = malloc(256);

    // don't remove this memset. not having this causes valgrind to freak out.
    memset(varstore->data_buf, 0, 256);

    varstore->next_offset = 0;
    varstore->data_buf_size = 256;
    return varstore;
}


void varstore_delete (struct varstore * varstore) {
    ODEL(varstore->tree);
    free(varstore->data_buf);
    free(varstore);
}


struct varstore * varstore_copy (const struct varstore * varstore) {
    struct varstore * new = malloc(sizeof(struct varstore));
    object_init(&(new->oh), &varstore_vtable);
    new->tree = OCOPY(varstore->tree);
    new->data_buf = malloc(varstore->data_buf_size);
    memcpy(new->data_buf, varstore->data_buf, varstore->data_buf_size);
    new->data_buf_size = varstore->data_buf_size;
    new->next_offset = varstore->next_offset;
    return new;
}


size_t varstore_insert (struct varstore * varstore,
                        const char * identifier,
                        size_t bits) {

    // figure out how much space to allocate
    size_t bytes = bits / 8;
    if (bytes < 4)
        bytes = 4;
    else if (bytes & 0x3) {
        bytes += 4;
        bytes &= (~0x3);
    }

    // make sure we have room in data_buf
    if (varstore->next_offset + bytes > varstore->data_buf_size) {
        varstore->data_buf = realloc(varstore->data_buf,
                                     varstore->data_buf_size * 2);
        // don't remove this memset. not having this causes valgrind to freak out.
        memset(&(varstore->data_buf), 0, varstore->data_buf_size);
        varstore->data_buf_size *= 2;

    }

    // drop this node into tree
    tree_insert_(varstore->tree, varstore_node_create(identifier,
                                                      bits,
                                                      varstore->next_offset));

    // return the offset to this variable
    size_t result = varstore->next_offset;
    varstore->next_offset += bytes;
    return result;
}


int varstore_offset (const struct varstore * varstore,
                     const char * identifier,
                     size_t bits,
                     size_t * offset) {
    struct varstore_node varstore_node;
    object_init(&(varstore_node.oh), &varstore_node_vtable);
    varstore_node.identifier = (char *) identifier;
    varstore_node.bits = bits;
    struct varstore_node * vn = tree_fetch(varstore->tree, &varstore_node);
    if (vn == NULL) {
        return -1;
    }
    else {
        *offset = vn->offset;
        return 0;
    }
}


int varstore_value (const struct varstore * varstore,
                    const char * identifier,
                    size_t bits,
                    uint64_t * value) {
    struct varstore_node varstore_node;
    object_init(&(varstore_node.oh), &varstore_node_vtable);
    varstore_node.identifier = (char *) identifier;
    varstore_node.bits = bits;
    struct varstore_node * vn = tree_fetch(varstore->tree, &varstore_node);
    if (vn == NULL)
        return -1;
    switch (bits) {
    case 1 :
        *value = *((uint8_t *) &(varstore->data_buf[vn->offset]));
        *value &= 1;
        break;
    case 8 :
        *value = *((uint8_t *) &(varstore->data_buf[vn->offset]));
        break;
    case 16 :
        *value = *((uint16_t *) &(varstore->data_buf[vn->offset]));
        break;
    case 32 :
        *value = *((uint32_t *) &(varstore->data_buf[vn->offset]));
        break;
    case 64 :
        *value = *((uint64_t *) &(varstore->data_buf[vn->offset]));
        break;
    return -1;
    }
    return 0;
}


void * varstore_data_buf (struct varstore * varstore) {
    return varstore->data_buf;
}


size_t varstore_offset_create (struct varstore * varstore,
                               const char * identifier,
                               size_t bits) {
    size_t offset = 0;
    if (varstore_offset(varstore, identifier, bits, &offset) == 0)
        return offset;
    return varstore_insert(varstore, identifier, bits);
}
