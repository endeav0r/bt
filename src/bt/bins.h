#ifndef bins_HEADER
#define bins_HEADER

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
    BOP_CALL,

    BOP_COMMENT
};


enum {
    BOPER_VARIABLE = 0,
    BOPER_CONSTANT
};


struct boper {
    struct object_header oh;
    unsigned int type;
    unsigned int bits;
    const char * identifier;
    uint64_t value;
};


struct bins {
    struct object_header oh;
    int op;
    union {
        struct boper * oper[3];
    };
};


struct boper * boper_create (unsigned int type,
                             unsigned int bits,
                             const char * identifier,
                             uint64_t value);
struct boper * boper_variable (unsigned int bits, const char * identifier);
struct boper * boper_constant (unsigned int bits, uint64_t value);
void           boper_delete   (struct boper * boper);
struct boper * boper_copy     (const struct boper * boper);
int            boper_cmp      (const struct boper * lhs,
                               const struct boper * rhs);

// caller must free string
char * boper_string (const struct boper * boper);
unsigned int boper_type       (const struct boper * boper);
const char * boper_identifier (const struct boper * boper);
unsigned int boper_bits       (const struct boper * boper);
uint64_t     boper_value      (const struct boper * boper);

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
struct bins * bins_ ## XXX (const struct boper * oper0, \
                            const struct boper * oper1, \
                            const struct boper * oper2); \
struct bins * bins_ ## XXX ## _ (struct boper * oper0, \
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
struct bins * bins_ ## XXX (const struct boper * oper0, \
                          const struct boper * oper1); \
struct bins * bins_ ## XXX ## _ (struct boper * oper0, \
                           struct boper * oper1);

BINS_2OP_DECL(sext)
BINS_2OP_DECL(zext)
BINS_2OP_DECL(trun)
BINS_2OP_DECL(load)
BINS_2OP_DECL(store)

struct bins * bins_hlt     ();
struct bins * bins_comment ();

#endif
