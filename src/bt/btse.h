#ifndef btse_HEADER
#define btse_HEADER

#include "memmap.h"
#include "tree.h"

#include <inttypes.h>

enum {
    BTSE_VAR_CONSTANT,
    BTSE_VAR_VARIABLE,
    BTSE_VAR_SYMBOLIC,
    BTSE_VAR_EXPRESSION
};

enum {
    BTSE_OP_ADD,
    BTSE_OP_SUB,
    BTSE_OP_UMUL,
    BTSE_OP_UDIV,
    BTSE_OP_UMOD,
    BTSE_OP_AND,
    BTSE_OP_OR,
    BTSE_OP_XOR,
    BTSE_OP_SHL,
    BTSE_OP_SHR,
    BTSE_OP_CMPEQ,
    BTSE_OP_CMPLTU,
    BTSE_OP_CMPLTS,
    BTSE_OP_CMPLEU,
    BTSE_OP_CMPLES,
    BTSE_OP_SEXT,
    BTSE_OP_ZEXT,
    BTSE_OP_TRUN,
    BTSE_OP_STORE,
    BTSE_OP_LOAD,
};

struct btse_var {
    struct object_header oh;
    char * identifier;
    unsigned int bits;
    unsigned int type;
    union {
        uint64_t value;
        unsigned int op;
    };
    struct btse_var * lhs;
    struct btse_var * rhs;
};

/**
* Creates a constant btse variable.
*
* @param bits Size of this constant in bits.
* @param value The value of this constant.
* @return An instantiated, initialized btse constant.
*/
struct btse_var * btse_var_constant (unsigned int bits, uint64_t value);

/**
* Creates a regular btse variable.
*
* @param identifier The identifier for this variable, for example "eax".
* @param bits The size of this variable in bits.
* @param value The value of this variable.
* @return An instantiated, initialized btse variable.
*/
struct btse_var * btse_var_variable (const char * identifier,
                                     unsigned int bits,
                                     uint64_t value);

/**
* Create a symbolic btse variable.
*
* @param identifier The identifier for this variable, for example "eax".
* @param bits The size of this variable in bits.
* @return An instantiated, initialized btse variable.
*/
struct btse_var * btse_var_symbolic (const char * identifier,
                                     unsigned int bits);

/**
* Create an expression btse variable. This is the form of the function which
* takes ownershit of the object arguments.
*
* @param op The op performed by this expression. One of BTSE_OP_*.
* @param bits The size of the expression's result in bits.
* @param identifier An optional identifier. This will be copied. If no
*                   identifier is necessary, this may be set to NULL.
* @param lhs The left-hand side of the expression. This will not be copied.
* @param rhs The right-hand side of the expression. This will not be copied.
* @return An instantiated, initialized btse expression.
*/
struct btse_var * btse_var_expression_ (unsigned int op,
                                        unsigned int bits,
                                        const char * identifier,
                                        struct btse_var * lhs,
                                        struct btse_var * rhs);
struct btse_var * btse_var_expression (unsigned int op,
                                       unsigned int bits,
                                       const char * identifier,
                                       const struct btse_var * lhs,
                                       const struct btse_var * rhs);
void              btse_var_delete (struct btse_var * btse_var);
struct btse_var * btse_var_copy   (const struct btse_var * btse_var);
int               btse_var_cmp    (const struct btse_var * lhs,
                                   const struct btse_var * rhs);

int          btse_var_set        (struct btse_var * btse_var, uint64_t value);
unsigned int btse_var_type       (const struct btse_var * btse_var);
const char * btse_var_identifier (const struct btse_var * btse_var);
unsigned int btse_var_bits       (const struct btse_var * btse_var);
uint64_t     btse_var_value      (const struct btse_var * btse_var);
int64_t      btse_var_sext       (const struct btse_var * btse_var);


struct btse {
    struct object_header oh;
    struct tree * vars;
    struct tree * symmem;
    struct memmap * memmap;
};


struct btse * btse_create (struct memmap * memmap);
void          btse_delete (struct btse * btse);
struct btse * btse_copy   (const struct btse * btse);

int btse_var_set_ (struct btse * btse, struct btse_var * bv);
int btse_var_set  (struct btse * btse, const struct btse_var * bv);

int btse_execute (struct btse * btse, struct bins * bins);

#endif
