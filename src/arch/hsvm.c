#include "arch/hsvm.h"

#include <stddef.h>

struct reg_code_table {
    unsigned int reg_code;
    const char * reg_string;
};


struct reg_code_table reg_codes [] = {
    {0x0, "r0"},
    {0x1, "r1"},
    {0x2, "r2"},
    {0x3, "r3"},
    {0x4, "r4"},
    {0x5, "r5"},
    {0x6, "r6"},
    {0xA, "r7"},
    {0x8, "rbp"},
    {0x9, "rsp"},
    {0x7, "rip"},
    {-1, NULL}
};


const char * hsvm_reg_string (unsigned int reg_code) {
    unsigned int i;
    for (i = 0; reg_codes[i].reg_string != NULL; i++) {
        if (reg_codes[i].reg_code == reg_code)
            return reg_codes[i].reg_string;
    }
    return NULL;
}
