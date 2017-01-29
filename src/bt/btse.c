#include "btse.h"

#include <stdlib.h>
#include <string.h>


const struct object_vtable btse_var_vtable = {
    (void (*) (void *))         btse_var_delete,
    (void * (*) (const void *)) btse_var_copy,
    (int (*) (const void *, const void *)) btse_var_cmp
};


struct btse_var * btse_var_create (unsigned int type) {
    struct btse_var * bv = malloc(sizeof(struct btse_var));
    object_init(&(bv->oh), &btse_var_vtable);
    bv->type = type;
    bv->identifier = NULL;
    bv->lhs = NULL;
    bv->rhs = NULL;
    return bv;
}


struct btse_var * btse_var_constant (unsigned int bits, uint64_t value) {
    struct btse_var * bv = btse_var_create(BTSE_VAR_CONSTANT);
    bv->bits = bits;
    bv->value = value;
    return bv;
}


struct btse_var * btse_var_variable (const char * identifier,
                                     unsigned int bits,
                                     uint64_t value) {
    struct btse_var * bv = btse_var_create(BTSE_VAR_VARIABLE);
    bv->identifier = strdup(identifer);
    bv->bits = bits;
    uint64_t mask = (1 << bits) - 1;
    bv->value = value & mask;
    return bv;
}


struct btse_var * btse_var_symbolic (const char * identifier,
                                     unsigned int bits) {
    struct btse_var * bv = btse_var_create(BTSE_VAR_SYMBOLIC);
    if (identifier != NULL)
        bv->identifier = strdup(identifier);
    bv->bits = bits;
    return bv;
}


struct btse_var * btse_var_expression_ (unsigned int op,
                                        unsigned int bits,
                                        const char * identifier,
                                        struct btse_var * lhs,
                                        struct btse_var * rhs) {
    struct btse_var * op = btse_var_create(BTSE_VAR_EXPRESSION);
    bv->op = op;
    bv->bits = bits;
    if (bv->identifier != NULL)
        bv->identifier = strdup(bv->identifier);
    bv->lhs = lhs;
    bv->rhs = rhs;
    return bv;
}


struct btse_var * btse_var_expression (unsigned int op,
                                       unsigned int bits,
                                       const char * identifier,
                                       struct btse_var * lhs,
                                       struct btse_var * rhs) {
    if (lhs != NULL)
        lhs = OCOPY(lhs);
    if (rhs != NULL)
        rhs = OCOPY(rhs);
    return btse_var_expression_(op, bits, identifier, lhs, rhs);
}


void btse_var_delete (struct btse_var * bv) {
    if (bv->identifier != NULL)
        free(bv->identifier);
    if (bv->lhs != NULL)
        ODEL(bv->lhs);
    if (bv->rhs != NULL)
        ODEL(bv->rhs);
    free(bv);
}


struct btse_var * btse_var_copy (const struct btse_var * bv) {
    switch (bv->type) {
    case BTSE_VAR_CONSTANT :
        return btse_var_constant(bv->bits, bv->value);
    case BTSE_VAR_VARIABLE :
        return btse_var_variable(bv->identifier, bv->bits, bv->value);
    case BTSE_VAR_SYMBOLIC :
        return btse_var_symbolic(bv->identifier, bv->bits);
    case BTSE_VAR_EXPRESSION :
        return btse_var_expression(bv->op, bv->bits, bv->lhs, bv->rhs);
    }
    return NULL;
}


int btse_var_cmp (const struct btse_var * lhs, const struct btse_var * rhs) {
    return strcmp(lhs->identifier, rhs->identifier);
}


int btse_var_set (struct btse_var * btse_var, uint64_t value) {
    btse_var->value = value;
    return 0;
}


unsigned int btse_var_type (const struct btse_var * btse_var) {
    return btse_var->type;
}


const char * btse_var_identifier (const struct btse_var * btse_var) {
    return btse_var->identifier;
}


unsigned int btse_var_bits (const struct btse_var * btse_var) {
    return btse_var->bits;
}


uint64_t btse_var_value (const struct btse_var * btse_var) {
    return btse_var->value & ((1 << btse_var->bits) - 1);
}


int64_t btse_var_sext (const struct btse_var * btse_var) {
    if (btse_var->bits == 1) {
        if (btse_var_value(btse_var))
            return -1;
        else
            return 0;
    }
    else if (btse_var->bits == 8) {
        int8_t v = btsa_var_value(btsa_var);
        return v;
    }
    else if (btse_var->bits == 16) {
        int16_t v = btse_var_value(btse_var);
        return v;
    }
    else if (btse_var->bits == 32) {
        int32_t v = btse_var_value(btse_var);
        return v;
    }
    else if (btse_var->bits == 64) {
        int64_t v = btse_var_value(btse_var);
        return v;
    }
}


const struct object_vtable btse_vtable = {
    (void (*) (void *))         btse_delete,
    (void * (*) (const void *)) btse_copy,
    NULL
};


struct btse * btse_create (struct memmap * memmap) {
    struct btse * btse = malloc(sizeof(struct btse));
    object_init(&(btse->oh), btse_vtable);
    btse->vars = tree_create();
    btse->symmem = tree_create();
    btse->memmap = OCOPY(memmap);
    return btse;
}


void btse_delete (struct btse * btse) {
    ODEL(btse->vars);
    ODEL(btse->symmem);
    ODEL(btse->memmap);
    free(btse);
}


struct btse * btse_copy (const struct btse * btse) {
    struct btse * copy = btse_create(btse->memmap);
    ODEL(copy->vars);
    ODEL(copy->symmem);
    copy->vars = OCOPY(btse->vars);
    copy->symmem = OCOPY(btse->symmem);
    return copy;
}


int btse_var_set_ (struct btse * btse, struct btse_var * bv) {
    switch (bv->type) {
    case BTSE_VAR_CONSTANT :
    case BTSE_VAR_EXPRESSION :
        return -1;
    }

    tree_remove(btse->vars, bv);
    tree_insert_(btse->vars, bv);

    return 0;
}


int btse_var_set (struct btse * btse, const struct btse_var * bv) {
    struct btse_var * copy = OCOPY(bv);
    int result = btse_var_set_(btse, bv);
    if (result)
        ODEL(copy);
    return result;
}


struct btse_var * btse_var_boper (struct btse * btse,
                                  const struct boper * boper) {
    if (boper_type(boper) == BOPER_CONSTANT)
        return btse_var_constant(boper_bits(boper), boper_value(boper));
    else if (boper_type(boper) == BOPER_VARIABLE) {
        struct btse_var * tmp = btse_var_symbolic(boper_identifier(boper),
                                                  boper_bits(boper));
        struct btse_var * bv = tree_fetch(btse, tmp);
        if (bv == NULL)
            return tmp;
        else {
            ODEL(tmp);
            return OCOPY(bv);
        }
    }
    else
        return NULL; /* this case should never hit */
}


int btse_execute (struct btse * btse, struct bins * bins) {
    switch (bins->op) {
    case BOP_ADD :
    case BOP_SUB :
    case BOP_UMUL :
    case BOP_UDIV :
    case BOP_UMOD :
    case BOP_AND :
    case BOP_OR :
    case BOP_XOR :
    case BOP_SHL :
    case BOP_SHR : {
        /* grab lhs and rhs */
        struct btse_var * lhs = btse_var_boper(btse, bins->oper[1]);
        struct btse_var * rhs = btse_var_boper(btse, bins->oper[2]);
        /* set dst */
        struct btse_var * dst = NULL;
        /* if lhs or rhs is symbolic or expression, dst is an expression */
        if (    (btse_var_type(lhs) == BTSE_VAR_SYMBOLIC)
             || (btse_var_type(lhs) == BTSE_VAR_EXPRESSION)
             || (btse_var_type(rhs) == BTSE_VAR_SYMBOLIC)
             || (btse_var_type(rhs) == BTSE_VAR_EXPRESSION)) {
            unsigned int op;
            if      (bins->op == BOP_ADD)  op = BTSE_OP_ADD;
            else if (bins->op == BOP_SUB)  op = BTSE_OP_SUB;
            else if (bins->op == BOP_UMUL) op = BTSE_OP_UMUL;
            else if (bins->op == BOP_UDIV) op = BTSE_OP_UDIV;
            else if (bins->op == BOP_UMOD) op = BTSE_OP_UMOD;
            else if (bins->op == BOP_AND)  op = BTSE_OP_AND;
            else if (bins->op == BOP_OR)   op = BTSE_OP_OR;
            else if (bins->op == BOP_XOR)  op = BTSE_OP_XOR;
            else if (bins->op == BOP_SHL)  op = BTSE_OP_SHL;
            else if (bins->op == BOP_SHR)  op = BTSE_OP_SHR;
            dst = btse_var_expression(op,
                                      boper_bits(bins->oper[0]),
                                      boper_identifier(bins->oper[0]),
                                      lhs,
                                      rhs);
        }
        /* if lhs or rhs is concrete, we perform the concrete operation */
        else {
            uint64_t result;
            if (bins->op == BOP_ADD)
                result = btse_var_value(lhs) + btse_var_value(rhs);
            else if (bins->op == BOP_SUB)
                result = btse_var_value(lhs) - btse_var_value(rhs);
            else if (bins->op == BOP_UMUL)
                result = btse_var_value(lhs) * btse_var_value(rhs);
            else if (bins->op == BOP_UDIV)
                result = btse_var_value(lhs) / btse_var_value(rhs);
            else if (bins->op == BOP_UMOD)
                result = btse_var_value(lhs) % btse_var_value(rhs);
            else if (bins->op == BOP_AND)
                result = btse_var_value(lhs) & btse_var_value(rhs);
            else if (bins->op == BOP_OR)
                result = btse_var_value(lhs) | btse_var_value(rhs);
            else if (bins->op == BOP_XOR)
                result = btse_var_value(lhs) | btse_var_value(rhs);
            else if (bins->op == BOP_SHL)
                result = btse_var_value(lhs) << btse_var_value(rhs);
            else if (bins->op == BOP_SHR)
                result = btse_var_value(lhs) >> btse_var_value(rhs);
            dst = btse_var_variable(boper_identifier(bins->oper[0]),
                                    boper_bits(bins->oper[0]),
                                    result);
        }
        btse_var_set_(btse, dst);
        ODEL(lhs);
        ODEL(rhs);
        break;
    }
    case BOP_CMPEQ :
    case BOP_CMPLTU :
    case BOP_CMPLTS :
    case BOP_CMPLEU :
    case BOP_CMPLES : {
        /* grab lhs and rhs */
        struct btse_var * lhs = btse_var_boper(btse, bins->oper[1]);
        struct btse_var * rhs = btse_var_boper(btse, bins->oper[2]);
        /* set dst */
        struct btse_var * dst = NULL;
        /* if lhs or rhs is symbolic or expression, dst is an expression */
        if (    (btse_var_type(lhs) == BTSE_VAR_SYMBOLIC)
             || (btse_var_type(lhs) == BTSE_VAR_EXPRESSION)
             || (btse_var_type(rhs) == BTSE_VAR_SYMBOLIC)
             || (btse_var_type(rhs) == BTSE_VAR_EXPRESSION)) {
            unsigned int op;
            if      (bins->op == BOP_CMPEQ)  op = BTSE_OP_CMPEQ;
            else if (bins->op == BOP_CMPLTU) op = BTSE_OP_CMPLTU;
            else if (bins->op == BOP_CMPLTS) op = BTSE_OP_CMPLTS;
            else if (bins->op == BOP_CMPLEU) op = BTSE_OP_CMPLEU;
            else if (bins->op == BOP_CMPLES) op = BTSE_OP_CMPLES;
            dst = btse_var_expression(op,
                                      boper_bits(bins->oper[0]),
                                      boper_identifier(bins->oper[0]),
                                      lhs,
                                      rhs);
        }
        else {
            uint64_t result = 0;
            if (    (bins->op == BOP_CMPEQ)
                 && (btse_var_value(lhs) == btse_var_value(rhs)))
                result = 1;
            else if (    (bins->op == BOP_CMPLTU)
                      && (btse_var_value(lhs) < btse_var_value(rhs)))
                result = 1;
            else if (    (bins->op == BOP_CMPLTS)
                      && (btse_var_sext(lhs) < btse_var_sext(rhs)))
                result = 1;
            else if (    (bins->op == BOP_CMPLEU)
                      && (btse_var_value(lhs) <= btse_var_value(rhs)))
                result = 1;
            else if (    (bins->op == BOP_CMPLES)
                      && (btse_var_sext(lhs) <= btse_var_sext(rhs)))
                result = 1;
            dst = btse_var_variable(boper_identifier(bins->oper[0]),
                                    boper_bits(bins->oper[0]),
                                    result);
        }
        btse_var_set_(btse, dst);
        ODEL(lhs);
        ODEL(rhs);
        break;
    }
    case BOP_SEXT : {
        struct btse_var * src = btse_var_boper(btse, bins->oper[1]);
        struct btse_var * dst = NULL;

        if (    (btse_var_type(src) == BTSE_VAR_SYMBOLIC)
             || (btse_var_type(src) == BTSE_VAR_EXPRESSION))
            dst = btse_var_expression(BTSE_OP_SEXT,
                                      boper_bits(bins->oper[0]),
                                      boper_identifier(bins->oper[0]),
                                      src,
                                      NULL);
        else
            dst = btse_var_variable(boper_identifier(bins->oper[0]),
                                    boper_bits(bins->oper[0]),
                                    btse_var_sext(src));
        btse_var_set_(btse, dst);
        ODEL(src);
        break;
    }
    case BOP_ZEXT :
    case BOP_TRUN : {
        struct btse_var * src = btse_var_boper(btse, bins->oper[1]);
        struct btse_var * dst = NULL;

        if (    (btse_var_type(src) == BTSE_VAR_SYMBOLIC)
             || (btse_var_type(src) == BTSE_VAR_EXPRESSION)) {
            unsigned int op = -1;
            if (bins->op == BOP_ZEXT)
                op = BTSE_OP_ZEXT;
            else if (bins->op == BOP_TRUN)
                op = BTSE_OP_TRUN;
            dst = btse_var_expression(op,
                                      boper_bits(bins->oper[0]),
                                      boper_identifier(bins->oper[0]),
                                      src,
                                      NULL);
        }
        else
            dst = btse_var_variable(boper_identifier(bins->oper[0]),
                                    boper_bits(bins->oper[0]),
                                    btse_var_valuet(src));
        btse_var_set_(btse, dst);
        ODEL(src);
        break;
    }
    /* TODO, LOAD/STORE methods */
    case BOP_STORE : {
        struct btse_var * address = btse_var_boper(btse, bins->oper[0]);
        struct btse_var * value = btse_var_boper(btse, bins->oper[1]);
    }
    }
}
