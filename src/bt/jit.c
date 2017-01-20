#include "jit.h"

#include "btlog.h"
#include "bt/bins.h"
#include "container/byte_buf.h"

#include <stdio.h>
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


const struct object jit_object = {
    (void (*) (void *))          jit_delete,
    (void * (*) (const void *))  jit_copy,
    NULL
};


struct jit * jit_create (const struct arch_source * arch_source,
                         const struct arch_target * arch_target) {
    struct jit * jit = malloc(sizeof(struct jit));

    jit->object = &jit_object;
    jit->blocks = tree_create();
    jit->mmap_mem = mmap(NULL,
                         INITIAL_MMAP_SIZE,
                         PROT_EXEC | PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS,
                         -1, 0);
    jit->mmap_size = INITIAL_MMAP_SIZE;
    jit->mmap_next = 0;

    jit->arch_source = arch_source;
    jit->arch_target = arch_target;

    return jit;
}


void jit_delete (struct jit * jit) {
    munmap(jit->mmap_mem, jit->mmap_size);
    ODEL(jit->blocks);
    free(jit);
}


struct jit * jit_copy (const struct jit * jit) {
    struct jit * copy = malloc(sizeof(struct jit));

    copy->object = &jit_object;
    copy->blocks = OCOPY(jit->blocks);
    copy->mmap_mem = mmap(NULL,
                          jit->mmap_size,
                          PROT_EXEC | PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1, 0);
    memcpy(copy->mmap_mem, jit->mmap_mem, jit->mmap_size);
    copy->mmap_size = jit->mmap_size;
    copy->mmap_size = jit->mmap_next;

    copy->arch_source = jit->arch_source;
    copy->arch_target = jit->arch_target;

    return copy;
}


int jit_set_code (struct jit * jit,
                  uint64_t vaddr,
                  const void * code,
                  size_t code_size) {
    memcpy(&(jit->mmap_mem[jit->mmap_next]), code, code_size);

    struct jit_block * jb = jit_block_create(vaddr, jit->mmap_next, code_size);
    tree_insert_(jit->blocks, jb);

    jit->mmap_next += (code_size + 0x100) & (~0xff);

    return 0;
}


const void * jit_get_code (struct jit * jit, uint64_t vaddr) {
    struct jit_block jb;
    jb.object = &jit_block_object;
    jb.vaddr = vaddr;
    struct jit_block * jit_block = tree_fetch(jit->blocks, &jb);
    if (jit_block == NULL)
        return NULL;
    return &(jit->mmap_mem[jit_block->mm_offset]);
}


int jit_execute (struct jit * jit,
                 struct varstore * varstore,
                 struct memmap * memmap) {
    // get the instruction pointer
    size_t offset;
    int error = varstore_offset(varstore,
                                jit->arch_source->ip_variable_identifier(),
                                jit->arch_source->ip_variable_bits(),
                                &offset);
    if (error)
        return -1;
    uint8_t * data_buf = (uint8_t *) varstore_data_buf(varstore);
    uint64_t ip;
    switch (jit->arch_source->ip_variable_bits()) {
    case 8 : ip = *((uint8_t *) &(data_buf[offset])); break;
    case 16 : ip = *((uint16_t *) &(data_buf[offset])); break;
    case 32 : ip = *((uint32_t *) &(data_buf[offset])); break;
    case 64 : ip = *((uint64_t *) &(data_buf[offset])); break;
    default: return -2;
    }

    // make sure memmap variable is set
    offset = varstore_offset_create(varstore, "__MEMMAP__", 64);
    *((uint64_t *) &(data_buf[offset])) = (uint64_t) memmap;

    // do we already have this block in the jit store?
    const void * codeptr = jit_get_code(jit, ip);
    btlog("[jit_execute.rip] %04x", ip);
    // we don't have this yet, jit it
    if (codeptr == NULL) {
        // get memory pointed to by instruction pointer
        struct buf * buf = memmap_get_buf(memmap, ip, 256);

        struct list * binslist;
        binslist = jit->arch_source->translate_block(buf_get(buf, 0, buf_length(buf)),
                                                     buf_length(buf));

        ODEL(buf);

        if (binslist == NULL)
            return -3;

        struct list_it * it;
        for (it = list_it(binslist); it != NULL; it = list_it_next(it)) {
            struct bins * bins = (struct bins *) list_it_data(it);

            char * str = bins_string(bins);
            btlog("[jit_execute.bins] %s", str);
            free(str);
        }

        // assemble instructions
        struct byte_buf * assembled_buf;
        assembled_buf = jit->arch_target->assemble(binslist, varstore);

        ODEL(binslist);

        if (assembled_buf == NULL)
            return -4;

        // set our rwx jit code
        jit_set_code(jit,
                     ip,
                     byte_buf_bytes(assembled_buf),
                     byte_buf_length(assembled_buf));

        ODEL(assembled_buf);
        codeptr = jit_get_code(jit, ip);
    }

    // execute this jit block
    unsigned int ret_code = jit->arch_target->execute(codeptr, varstore);

    return ret_code;
}
