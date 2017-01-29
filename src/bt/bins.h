#ifndef bins_HEADER
#define bins_HEADER

/**
* Bins is short for "Binary Toolkit Instruction," and is the basic IR most of
* the bt functionality operates off of.
*
* Boper are the operands for bins. Boper is short for, "Binary Toolkit Operand."
*/


#include "container/list.h"
#include "object.h"

#include <stdint.h>

enum {
    /* Arithmetic instructions */
    /* oper[0] = oper[1] OP oper[2] */
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

    /* Comparison instructions */
    /* oper[0] = oper[1] OP oper[2] ? 1 : 0 */
    BOP_CMPEQ,
    BOP_CMPLTU,
    BOP_CMPLTS,
    BOP_CMPLEU,
    BOP_CMPLES,

    /* Instructions for modifying the length of operands */
    /* oper[0] = oper[1] sign-extended to fit oper[0] bits */
    BOP_SEXT,
    /* oper[0] = oper[1] zero-extended to fit oper[0] bits */
    BOP_ZEXT,
    /* oper[0] = oper[1] truncated to fit oper[0] bits */
    BOP_TRUN,

    /* Memory read/write instructions */
    /* stores 8-bit value oper[1] in the address given by oper[0] */
    BOP_STORE,
    /* loads 8-bit value at address given by oper[1] into variable given by
       oper[1] */
    BOP_LOAD,

    /* HLT instruction */
    BOP_HLT,

    /* Auxiliary instructions with no semantic meaning */
    BOP_COMMENT,
    BOP_HOOK
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
    struct boper * oper[3];
    void (* hook) (void *);
};


/**
* Creates a boper. You should not call this function directly, but instead call
* boper_variable or boper_constant, which will in turn call this function.
* @param type The type of the boper.
* @param bits The size of the boper in bits.
* @param identifier The identifier if the boper, if required.
* @param value The value of the boper, if required.
* @return An instantiated and initialized boper.
*/
struct boper * boper_create (unsigned int type,
                             unsigned int bits,
                             const char * identifier,
                             uint64_t value);

/**
* Creates a boper variable.
* @param bits The size of the variable in bits.
* @param identifier The textual identifier of the variable.
* @return The resulting boper variable.
*/
struct boper * boper_variable (unsigned int bits, const char * identifier);

/**
* Creates a boper constant.
* @param bits The size of the constant in bits.
* @param value The value of the constant.
* @return The resulting boper constant.
*/
struct boper * boper_constant (unsigned int bits, uint64_t value);

/**
* Deletes a boper. You should not call this, call ODEL() instead.
* @param boper The boper to delete.
*/
void boper_delete (struct boper * boper);

/**
* Copies a boper. You should not call this, call OCOPY() instead.
* @param boper The boper to copy.
* @return A copy of the boper.
*/
struct boper * boper_copy (const struct boper * boper);

/**
* Compares a boper based on the boper's identifier. Allows bopers to be added
* to containers which require a cmp method, such as trees.
* @param lhs The left-hand side of the comparison.
* @param rhs The right-hand size of the comparison.
* @return -1 if lhs < rhs, 1 if lhs > rhs, or 0 if lhs == rhs.
*/
int boper_cmp (const struct boper * lhs, const struct boper * rhs);

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
struct bins * bins_hook    (void (* hook) (void *));

#endif
