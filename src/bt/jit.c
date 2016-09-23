#include "jit.h"

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

const struct object jit_block_object = {
    (void (*) (void *))                    jit_block_delete,
    (void * (*) (const void *))            jit_block_copy,
    (int (*) (const void *, const void *)) jit_block_cmp
};


struct jit_block * jit_block_create (uint64_t vaddr,
                                     size_t mm_offset,
                                     size_t size) {
    struct jit_block * jit_block = malloc(sizeof(struct jit_block));

    jit_block->object = &jit_block_object;
    jit_block->vaddr = vaddr;
    jit_block->mm_offset = mm_offset;
    jit_block->size = size;

    return jit_block;
}


void jit_block_delete (struct jit_block * jit_block) {
    free(jit_block);
}


struct jit_block * jit_block_copy (const struct jit_block * jit_block) {
    return jit_block_create(jit_block->vaddr,
                            jit_block->mm_offset,
                            jit_block->size);
}


int jit_block_cmp (const struct jit_block * lhs, const struct jit_block * rhs) {
    if (lhs->vaddr < rhs->vaddr)
        return -1;
    else if (lhs->vaddr > rhs->vaddr)
        return 1;
    return 0;
}


const struct object jit_var_object = {
    (void (*) (void *))                    jit_var_delete,
    (void * (*) (const void *))            jit_var_copy,
    (int (*) (const void *, const void *)) jit_var_cmp
};


struct jit_var * jit_var_create (const char * identifier,
                                 size_t offset,
                                 size_t size) {
    struct jit_var * jit_var = malloc(sizeof(struct jit_var));

    jit_var->object = &jit_var_object;
    jit_var->identifier = strdup(identifier);
    jit_var->offset = offset;
    jit_var->size = size;
    
    return jit_var;
}


void jit_var_delete (struct jit_var * jit_var) {
    free(jit_var->identifier);
    free(jit_var);
}


struct jit_var * jit_var_copy (const struct jit_var * jit_var) {
    return jit_var_create(jit_var->identifier, jit_var->offset, jit_var->size);
}


int jit_var_cmp (const struct jit_var * lhs, const struct jit_var * rhs) {
    int c = strcmp(lhs, rhs);
    if (c < 0)
        return -1;
    else if (c > 0)
        return 1;
    if (lhs->size < rhs->size)
        return -1;
    else if (lhs->size > rhs->size)
        return 1;
    return 0;
}


const struct object jit_object = {
    (void (*) (void *))                    jit_delete,
    (void * (*) (const void *))            jit_copy,
    (int (*) (const void *, const void *)) jit_cmp
};


struct jit * jit_create () {
    struct jit * jit = malloc(sizeof(struct jit));

    jit->object = &jit_object;
    jit->blocks = tree_create();
    jit->mmap_mem = mmap(NULL,
                         INITIAL_MMAP_SIZE,
                         PROT_EXEC | PROT_READ | PROT_WRITE,
                         MAP_PRIVATE,
                         MAP_ANONYMOUS,
                         -1, 0);
    jit->mmap_size = INITIAL_MMAP_SIZE;
    jit->variables = tree_create();
    jit->var_mem = malloc(INITIAL_VAR_MEM_SIZE);
    jit->var_mem_size = INITIAL_VAR_MEM_SIZE;
    jit->var_mem_top = 0;

    return jit;
}


void jit_delete (struct jit * jit) {
    munmap(jit->mmmap_mem, jit->mmap_size);
    ODEL(jit->blocks);
    ODEL(jit->variables);
    free(jit->var_mem);
    free(jit);
}


struct jit * jit_copy (const struct jit * jit) {
    struct jit * copy = malloc(sizeof(struct jit));

    copy->object = &jit_object;
    copy->blocks = OCOPY(jit->blocks);
    copy->mmap_mem = mmap(NULL,
                          jit->mmap_size,
                          PROT_EXEC | PROT_READ | PROT_WRITE,
                          MAP_PRIVATE,
                          MAP_ANONYMOUS,
                          -1, 0);
    memcpy(copy->mmap_mem, jit->mmap_mem, jit->mmap_size);
    copy->mmap_size = jit->mmap_size;
    copy->variables = OCOPY(jit->variables);
    copy->var_mem = malloc(jit->var_mem_size);
    memcpy(copy->var_mem, jit->var_mem, jit->var_mem_size);
    copy->var_mem_size = jit->var_mem_size;
    copy->var_mem_top = jit->var_mem_stop;

    return copy;
}