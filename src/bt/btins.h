#ifndef btil_HEADER
#define btil_HEADER

#include "container/list.h"
#include "object.h"

#include <stdint.h>

enum {
    BOP_ADD = 0,
    BOP_SUB,
    BOP_UMUL,
    BOP_UDIV,
    BOP_UMOD,
    BOP_AND,
    BOP_OR,
    BOP_XOR,
    BOP_SHL,
    BOP_SHR,
    BOP_CMPEQ,
    BOP_CMPLTU,
    BOP_CMPLTS,
    BOP_CMPLEU,
    BOP_CMPLES,
    BOP_SEXT,
    BOP_ZEXT,
    BOP_TRUN,
    BOP_STORE,
    BOP_LOAD,

    BOP_HLT,
    BOP_CALL
};


enum {
    BOPER_VARIABLE = 0,
    BOPER_CONSTANT
};


struct boper {
    const struct object * object;
    unsigned int type;
    unsigned int bits;
    const char * identifier;
    uint64_t value;
};


struct bins {
    const struct object * object;
    int op;
    union {
        struct boper * oper[3];
        struct list * (* call) (void * data);
    }
};


struct boper * boper_create (unsigned int type,
                             unsigned int bits,
                             const char * identifier,
                             uint64_t value);
struct boper * boper_variable (unsigned int bits, const char * identifier);
struct boper * boper_constant (unsigned int bits, uint64_t value);
void           boper_delete   (struct boper * boper);
struct boper * boper_copy     (const struct boper * boper);

char * boper_string (const struct boper * boper);

struct bins *  bins_create (int op,
                            const struct boper * oper0,
                            const struct boper * oper1,
                            const struct boper * oper2);
struct bins *  bins_create_ (int op,
                             struct boper * oper0,
                             struct boper * oper1,
                             struct boper * oper2);
void          bins_delete (struct bins * bins);
struct bins * bins_copy   (const struct bins * bins);

char * bins_string (const struct bins * bins);

#define BINS_3OP_DECL(XXX) \
struct bins * bins_#XXX (const struct boper * oper0, \
                         const struct boper * oper1, \
                         const struct boper * oper2); \
struct bins * bins_#XXX#_ (struct boper * oper0, \
                           struct boper * oper1, \
                           struct boper * oper2);
BINS_3OP_DECL(add)
BINS_3OP_DECL(sub)
BINS_3OP_DECL(umul)
BINS_3OP_DECL(udiv)
BINS_3OP_DECL(umod)
BINS_3OP_DECL(and)
BINS_3OP_DECL(or)
BINS_3OP_DECL(xor)
BINS_3OP_DECL(shl)
BINS_3OP_DECL(shr)
BINS_3OP_DECL(cmpeq)
BINS_3OP_DECL(cmpltu)
BINS_3OP_DECL(cmplts)
BINS_3OP_DECL(cmpleu)
BINS_3OP_DECL(cmples)

#define BINS_2OP_DECL(XXX) \
struct bins * bins_#XXX (const struct boper * oper0, \
                         const struct boper * oper1); \
struct bins * bins_#XXX#_ (struct boper * oper0, \
                           struct boper * oper1);
BINS_2OP_DECL(sext)
BINS_3OP_DECL(zext)
BINS_3OP_DECL(load)
BINS_3OP_DECL(store)

struct bins * bins_hlt  ();
struct bins * bins_call (void (* bcall) (void * data));

#endif