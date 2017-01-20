#include "target.h"

#include <stdlib.h>
#include <string.h>

const struct object target_variable_object = {
    (void (*) (void *)) target_variable_delete,
    (void * (*) (const void *)) target_variable_copy,
    (int (*) (const void *, const void *)) target_variable_cmp
};


struct target_variable * target_variable_create (
    const char * identifier,
    size_t offset,
    unsigned int bits
) {
    struct target_variable * tv = malloc(sizeof(struct target_variable));
    tv->object = &target_variable_object;
    tv->identifier = strdup(identifier);
    tv->offset = offset;
    tv->bits = bits;
    return tv;
}


void target_variable_delete (struct target_variable * tv) {
    free(tv->identifier);
    free(tv);
}


struct target_variable * target_variable_copy (
    const struct target_variable * tv
) {
    return target_variable_create(tv->identifer, tv->offset, tv->bits);
}


int target_variable_cmp (
    const struct target_variable * lhs,
    const struct target_variable * rhs
) {
    return strcmp(lhs->identifier, rhs->identifier);
}



const struct object target_codeblock_object = {
    (void (*) (void *)) target_codeblock_delete,
    (void * (*) (const void *)) target_codeblock_copy,
    (int (*) (const void *, const void *)) target_codeblock_cmp
};


struct target_codeblock * target_codeblock_create (
    uint64_t virtual_address,
    size_t code_size,
    size_t rwx_offset,
    size_t rwx_code_size
) {
    struct target_codeblock * tc = malloc(sizeof(struct target_codeblock));
    tc->object = &target_codeblock_object;
    tc->virtual_address = virtual_address;
    tc->code_size = code_size;
    tc->rwx_offset = rwx_offset;
    tc->rwx_code_size = rwx_code_size;
    return tc;
}


void target_codeblock_delete (struct target_codeblock * tc) {
    free(tc);
}


struct target_codeblock * target_codeblock_copy (
    const struct target_codeblock * tc
) {
    return target_codeblock_create(tc->virtual_address,
                                   tc->code_size,
                                   tc->rwx_offset,
                                   tc->rwx_code_size);
}


int target_codeblock_cmp (
    const struct target_codeblock * lhs,
    const struct target_codeblock * rhs
) {
    if (lhs->virtual_address < rhs->virtual_address)
        return -1;
    else if (lhs->virtual_address > rhs->virtual_address)
        return 1;
    return 0;
}


const struct object target_object = {
    (void (*) (void *)) target_delete,
    (void * (*) (const void *)) target_copy,
    NULL
};


struct target * target_create (struct mmap * mmap_) {
    struct target * target = malloc(sizeof(struct target));
    target->object = &target_object;
    target->variable_space = malloc(1024);
    target->variable_space_size = 1024;
    target->variables = tree_create();
    target->codeblocks = tree_create();
    target->rwx_mem = mmap(NULL,
                           1024 * 1024 * 32,
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_ANONYMOUS | MAP_PRIVATE,
                           -1,
                           0);
    target->rwx_mem_size - 1024 * 1024 * 32;
    target->mmap_ = OCOPY(mmap_);
    return target;
}


void target_delete (struct target * target) {
    free(target->variable_space);
    ODEL(target->variables);
    ODEL(target->codeblocks);
    munmap(target->rwx_mem, target->rwx_mem_size);
    free(target);
}


struct target * target_copy (const struct target * target) {
    struct target * new = malloc(sizeof(struct target));
    new->object = &target_object;

    new->variable_space_size = target->variable_space_size;
    new->variable_space = malloc(new->variable_space_size);
    memcpy(new->variable_space, target->variable_space, new->variable_space_size);

    new->variables = OCOPY(target->variables);
    new->codeblocks = OCOPY(target->codeblocks);

    new->rwx_mem = mmap(NULL,
                        target->rwx_mem_size,
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_ANONYMOUS | MAP_PRIVATE,
                        -1,
                        0);
    memcpy(new->rwx_mem, target->rwx_mem, target->rwx_mem_size);
    new->rwx_mem_size = target->rwx_mem_size;

    new->mmap = OCOPY(target->mmap);

    return new;
}
