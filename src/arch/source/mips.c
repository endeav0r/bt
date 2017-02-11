#include "arch/source/mips.h"

#include "arch/mips.h"
#include "bt/bins.h"
#include "container/list.h"

#include <stdlib.h>

#include <capstone/capstone.h>



const struct arch_source arch_source_mips = {
    mips_ip_variable_identifier,
    mips_ip_variable_bits,
    mips_translate_ins,
    mips_translate_block
};


struct boper * mips_boper_register (unsigned int reg) {
    const char * reg_string = mips_reg_string(reg);
    if (reg_string == NULL)
        return NULL;
    else
        return boper_variable(32, reg_string);
}


const char * mips_ip_variable_identifier () { return "$pc"; }


unsigned int mips_ip_variable_bits () { return 32; }


struct boper * mips_integer_overflow (struct list * binslist,
                                      const struct boper * lhs,
                                      const struct boper * rhs) {
    list_append_(binslist, bins_add_(boper_variable(32, "io_tmp"),
                                     OCOPY(lhs),
                                     OCOPY(rhs)));
    list_append_(binslist, bins_cmpltu_(boper_variable(1, "io_flag"),
                                        boper_variable(32, "io_tmp"),
                                        OCOPY(lhs)));
    return boper_variable(1, "io_flag");
}


struct list * mips_translate_ins (const void * buf, size_t size) {
    csh handle;
    cs_insn * insn;
    size_t count;

    if (cs_open(CS_ARCH_MIPS, CS_MODE_MIPS32 | CS_MODE_BIG_ENDIAN, &handle) != CS_ERR_OK)
        return NULL;
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    count = cs_disasm(handle, buf, size, 0, 1, &insn);
    if (count < 1) {
        cs_close(&handle);
        return NULL;
    }

    const struct cs_mips_op * operands = insn->detail->mips.operands;

    struct list * binslist = list_create();

    list_append_(binslist, bins_add_(boper_variable(32, "$pc"),
                                     boper_variable(32, "$pc"),
                                     boper_constant(32, 4)));

    switch (insn->id) {
    case MIPS_INS_ADD :
        list_append_(binslist, bins_add_(boper_variable(32, "io_tmp"),
                                         mips_boper_register(operands[1].reg),
                                         mips_boper_register(operands[2].reg)));
        list_append_(binslist, bins_cmpltu_(boper_variable(1, "io_flag"),
                                            boper_variable(32, "io_tmp"),
                                            mips_boper_register(operands[1].reg)));
        list_append_(binslist, bins_or_(boper_variable(32, "halt_code"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 1)));
        list_append_(binslist, bins_ce_(boper_variable(1, "io_flag")));
        list_append_(binslist, bins_hlt());
        list_append_(binslist, bins_add_(mips_boper_register(operands[0].reg),
                                         mips_boper_register(operands[1].reg),
                                         mips_boper_register(operands[2].reg)));
        break;
    case MIPS_INS_ADDI :
        list_append_(binslist, bins_add_(boper_variable(32, "io_tmp"),
                                         mips_boper_register(operands[1].reg),
                                         boper_constant(32, operands[2].imm)));
        list_append_(binslist, bins_cmpltu_(boper_variable(1, "io_flag"),
                                            boper_variable(32, "io_tmp"),
                                            mips_boper_register(operands[1].reg)));
        list_append_(binslist, bins_or_(boper_variable(32, "halt_code"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 1)));
        list_append_(binslist, bins_ce_(boper_variable(1, "io_flag")));
        list_append_(binslist, bins_hlt());
        list_append_(binslist, bins_add_(mips_boper_register(operands[0].reg),
                                         mips_boper_register(operands[1].reg),
                                         boper_constant(32, operands[2].imm)));
        break;
    case MIPS_INS_AND :
        list_append_(binslist, bins_and_(mips_boper_register(operands[0].reg),
                                         mips_boper_register(operands[1].reg),
                                         mips_boper_register(operands[2].reg)));
        break;
    case MIPS_INS_ANDI :
        list_append_(binslist, bins_and_(mips_boper_register(operands[0].reg),
                                         mips_boper_register(operands[1].reg),
                                         boper_constant(32, operands[2].imm & 0xffff)));
        break;
    case MIPS_INS_BEQ :
        list_append_(binslist, bins_cmpeq_(boper_variable(1, "flag"),
                                           mips_boper_register(operands[0].reg),
                                           mips_boper_register(operands[1].reg)));
        list_append_(binslist, bins_sext_(boper_variable(32, "flag32"),
                                          boper_variable(1, "flag")));
        list_append_(binslist, bins_and_(boper_variable(32, "branch_immediate"),
                                         boper_constant(32, operands[2].imm),
                                         boper_variable(32, "flag32")));
        list_append_(binslist, bins_add_(boper_variable(32, "$pc"),
                                         boper_variable(32, "$pc"),
                                         boper_variable(32, "branch_immeidate")));
        break;
    }

    cs_free(insn, count);
    cs_close(&handle);

    return binslist;
}


struct list * mips_translate_block (const void * buf, size_t size) {
    return mips_translate_ins(buf, size);
}
