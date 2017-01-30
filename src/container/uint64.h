#ifndef uint64_HEADER
#define uint64_HEADER

/**
* Sometimes we need an object which just holds a uint64. This is that object.
*/

#include "object.h"

#include <stdint.h>
#include <stdlib.h>

struct uint64 {
    struct object_header oh;
    uint64_t value;
};


/**
* Create a uint64 object.
* @return An initialized uint64 object.
*/
struct uint64 * uint64_create (uint64_t value);

/**
* Delete a uint64 object. You should not call this, call ODEL()
* @param uint64 The uint64 to delete.
*/
void uint64_delete (struct uint64 * uint64);

/**
* Copy a uint64 object. You should not call this, call OCOPY()
* @param uint64 A pointer to the uint64 to copy.
* @return A uint64 of the given uint64.
*/
struct uint64 * uint64_copy (const struct uint64 * uint64);

/**
* Compare two uint64 objects.
* @param lhs The left-hand side of the equation.
* @param rhs The right-hand side of the equation.
* @return -1 if lhs->value < rhs->value, 1 if lhs->value > rhs->value,
          0 if lhs->value == rhs->value
*/
int uint64_cmp (const struct uint64 * lhs, const struct uint64 * rhs);

#endif
