#include "bins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bins_string {
    int op;
    const char * string;
};

const struct bins_string bins_strings [] = {
    {BOP_ADD,  "add"},
    {BOP_SUB,  "sub"},
    {BOP_UMUL, "umul"},
    {BOP_UDIV, "udiv"},
    {BOP_AND,  "and"},
    {BOP_OR,   "or"},
    {BOP_XOR,  "xor"},
    {BOP_SHL,  "shl"},
    {BOP_SHR,  "shr"},
    {BOP_CMPEQ, "cmpeq"},
    {BOP_CMPLTU, "cmpltu"},
    {BOP_CMPLTS, "cmplts"},
    {BOP_CMPLEU, "cmpleu"},
    {BOP_CMPLES, "cmples"},
    {BOP_SEXT, "sext"},
    {BOP_ZEXT, "zext"},
    {BOP_TRUN, "trun"},
    {BOP_STORE, "store"},
    {BOP_LOAD, "load"},
    {-1, NULL}
};

const struct object boper_object = {
    (void (*) (void *)) boper_delete,
    (void * (*) (const void *)) boper_copy,
    (int (*) (const void *, const void *)) boper_cmp
};


struct boper * boper_create (unsigned int type,
                             unsigned int bits,
                             const char * identifier,
                             uint64_t value) {
    struct boper * boper = malloc(sizeof(struct boper));

    boper->object = &boper_object;
    boper->type = type;
    boper->bits = bits;
    if (identifier != NULL)
        boper->identifier = strdup(identifier);
    else
        boper->identifier = NULL;
    boper->value = value;

    return boper;
}


struct boper * boper_variable (unsigned int bits, const char * identifier) {
    return boper_create(BOPER_VARIABLE, bits, identifier, 0);
}


struct boper * boper_constant (unsigned int bits, uint64_t value) {
    return boper_create(BOPER_CONSTANT, bits, NULL, value);
}


void boper_delete (struct boper * boper) {
    if (boper->identifier != NULL)
        free((void *) boper->identifier);
    free(boper);
}


struct boper * boper_copy (const struct boper * boper) {
    return boper_create(boper->type,
                        boper->bits,
                        boper->identifier,
                        boper->value);
}


int boper_cmp (const struct boper * lhs, const struct boper * rhs) {
    if (lhs->type < rhs->type)
        return -1;
    else if (lhs->type > rhs->type)
        return 1;
    else if (lhs->bits < rhs->bits)
        return -1;
    else if (lhs->bits > rhs->bits)
        return 1;
    else if (lhs->type == BOPER_CONSTANT) {
        if (boper_value(lhs) < boper_value(rhs))
            return -1;
        else if (boper_value(lhs) > boper_value(rhs))
            return 1;
        return 0;
    }
    return strcmp(lhs->identifier, rhs->identifier);
}


char * boper_string (const struct boper * boper) {
    char * s = NULL;
    if (boper->type == BOPER_VARIABLE) {
        size_t size = strlen(boper->identifier);
        s = malloc(size + 32);
        snprintf(s, size, "%s:%u", boper->identifier, boper->bits);
    }
    else if (boper->type == BOPER_CONSTANT) {
        s = malloc(64);
        snprintf(s, 64, "0x%llx:%u", boper_value(boper), boper->bits);
    }
    return s;
}

unsigned int boper_type (const struct boper * boper) {
    return boper->type;
}

const char * boper_identifier (const struct boper * boper) {
    return boper->identifier;
}

unsigned int boper_bits (const struct boper * boper) {
    return boper->bits;
}

uint64_t boper_value (const struct boper * boper) {
    if (boper->bits == 64)
        return boper->value;
    return boper->value & ((1 << boper->bits) - 1);
}

const struct object bins_object = {
    (void (*) (void *)) bins_delete,
    (void * (*) (const void *)) bins_copy,
    NULL
};


struct bins * bins_create (int op,
                           const struct boper * oper0,
                           const struct boper * oper1,
                           const struct boper * oper2) {
    struct bins * bins = malloc(sizeof(struct bins));
    bins->object = &bins_object;
    bins->op = op;
    if (oper0)
        bins->oper[0] = OCOPY(oper0);
    else
        bins->oper[0] = NULL;
    if (oper1)
        bins->oper[1] = OCOPY(oper1);
    else
        bins->oper[1] = NULL;
    if (oper2)
        bins->oper[2] = OCOPY(oper2);
    else
        bins->oper[2] = NULL;
    return bins;
}


struct bins * bins_create_ (int op,
                            struct boper * oper0,
                            struct boper * oper1,
                            struct boper * oper2) {
    struct bins * bins = malloc(sizeof(struct bins));
    bins->op = op;

    bins->oper[0] = oper0;
    bins->oper[1] = oper1;
    bins->oper[2] = oper2;

    return bins;
}


void bins_delete (struct bins * bins) {
    if (bins->op != BOP_CALL) {
        if (bins->oper[0]) ODEL(bins->oper[0]);
        if (bins->oper[1]) ODEL(bins->oper[1]);
        if (bins->oper[2]) ODEL(bins->oper[2]);
    }

    free(bins);
}


char * bins_string (const struct bins * bins) {
    const char * op_string = NULL;
    unsigned int i;
    for (i = 0; bins_strings[i].string != NULL; i++) {
        if (bins_strings[i].op == bins->op) {
            op_string = bins_strings[i].string;
            break;
        }
    }

    if (op_string == NULL)
        return NULL;

    char * s = NULL;
    switch (bins->op) {
    case BOP_ADD :
    case BOP_SUB :
    case BOP_UMUL :
    case BOP_UDIV :
    case BOP_AND :
    case BOP_OR :
    case BOP_XOR :
    case BOP_SHL :
    case BOP_SHR :
    case BOP_CMPEQ :
    case BOP_CMPLTU :
    case BOP_CMPLTS :
    case BOP_CMPLEU :
    case BOP_CMPLES : {
        s = malloc(128);
        char * o0str = boper_string(bins->oper[0]);
        char * o1str = boper_string(bins->oper[1]);
        char * o2str = boper_string(bins->oper[2]);
        snprintf(s, 128, "%s %s, %s, %s", op_string, o0str, o1str, o2str);
        s[127] = '\0';
        free(o0str);
        free(o1str);
        free(o2str);
        break;
    }
    case BOP_SEXT :
    case BOP_ZEXT :
    case BOP_TRUN :
    case BOP_STORE :
    case BOP_LOAD : {
        s = malloc(128);
        char * o0str = boper_string(bins->oper[0]);
        char * o1str = boper_string(bins->oper[1]);
        snprintf(s, 128, "%s %s, %s", op_string, o0str, o1str);
        s[127] = '\0';
        free(o0str);
        free(o1str);
        break;
    }
    }

    return s;
}


struct bins * bins_copy (const struct bins * bins) {
    return bins_create(bins->op, bins->oper[0], bins->oper[1], bins->oper[2]);
}


#define BINS_3OP_DEF(XXX, YYY) \
struct bins * bins_##XXX (const struct boper * oper0, \
                          const struct boper * oper1, \
                          const struct boper * oper2) { \
    return bins_create(BOP_##YYY, oper0, oper1, oper2); \
} \
struct bins * bins_##XXX_ (struct boper * oper0, \
                           struct boper * oper1, \
                           struct boper * oper2) { \
    return bins_create_(BOP_##YYY, oper0, oper1, oper2); \
}

BINS_3OP_DEF(add, ADD)


#define BINS_2OP_DEF(XXX, YYY) \
struct bins * bins_##XXX (const struct boper * oper0, \
                         const struct boper * oper1) { \
    return bins_create(BOP_#YYY, oper0, oper1, NULL); \
} \
struct bins * bins_##XXX_ (struct boper * oper0, \
                           struct boper * oper1) { \
    return bins_create_(BOP_#YYY, oper0, oper1, NULL); \
}


struct bins * bins_hlt () {
    return bins_create(BOP_HLT, NULL, NULL, NULL);
}