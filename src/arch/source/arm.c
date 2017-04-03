#include "arch/source/arm.h"

#include "btlog.h"
#include "bt/bins.h"

#include <vex/libvex.h>

const struct arch_source arch_source_arm = {
    arm_ip_variable_identifier,
    arm_ip_variable_bits,
    arm_translate_ins,
    arm_translate_block
};


const char * arm_ip_variable_identifier () {
    return "pc";
}


unsigned int arm_ip_variable_bits () {
    return 32;
}


struct asarm_cs_reg_table_entry {
    arm_reg cs_reg;
    const char * name;
    unsigned char bits;
};


struct asarm_cs_reg_table_entry asarm_cs_regs [] = {
    {ARM_REG_LR, "lr", 32},
    {ARM_REG_PC, "pc", 32},
    {ARM_REG_SP, "sp", 32},
    {ARM_REG_D0, "d0", 64},
    {ARM_REG_D1, "d1", 64},
    {ARM_REG_D2, "d2", 64},
    {ARM_REG_D3, "d3", 64},
    {ARM_REG_D4, "d4", 64},
    {ARM_REG_D5, "d5", 64},
    {ARM_REG_D6, "d6", 64},
    {ARM_REG_D7, "d7", 64},
    {ARM_REG_D8, "d8", 64},
    {ARM_REG_D9, "d9", 64},
    {ARM_REG_D10, "d10", 64},
    {ARM_REG_D11, "d11", 64},
    {ARM_REG_D12, "d12", 64},
    {ARM_REG_D13, "d13", 64},
    {ARM_REG_D14, "d14", 64},
    {ARM_REG_D15, "d15", 64},
    {ARM_REG_D16, "d16", 64},
    {ARM_REG_D17, "d17", 64},
    {ARM_REG_D18, "d18", 64},
    {ARM_REG_D19, "d19", 64},
    {ARM_REG_D20, "d20", 64},
    {ARM_REG_D21, "d21", 64},
    {ARM_REG_D22, "d22", 64},
    {ARM_REG_D23, "d23", 64},
    {ARM_REG_D24, "d24", 64},
    {ARM_REG_D25, "d25", 64},
    {ARM_REG_D26, "d26", 64},
    {ARM_REG_D27, "d27", 64},
    {ARM_REG_D28, "d28", 64},
    {ARM_REG_D29, "d29", 64},
    {ARM_REG_D30, "d30", 64},
    {ARM_REG_D31, "d31", 64},
    {ARM_REG_R0, "r0", 32},
    {ARM_REG_R1, "r1", 32},
    {ARM_REG_R2, "r2", 32},
    {ARM_REG_R3, "r3", 32},
    {ARM_REG_R4, "r4", 32},
    {ARM_REG_R5, "r5", 32},
    {ARM_REG_R6, "r6", 32},
    {ARM_REG_R7, "r7", 32},
    {ARM_REG_R8, "r8", 32},
    {ARM_REG_R9, "r9", 32},
    {ARM_REG_R10, "r10", 32},
    {ARM_REG_R11, "r11", 32},
    {ARM_REG_R12, "r12", 32},
    {-1, NULL, -1}
};


struct boper * asarm_cs_reg (unsigned int cs_reg) {
    unsigned int i;
    for (i = 0; asarm_cs_regs[i].name != NULL; i++) {
        if (asarm_cs_regs[i].cs_reg == cs_reg)
            return boper_variable(
                asarm_cs_regs[i].bits,
                asarm_cs_regs[i].name
            );
    }
    return NULL;
}


struct list * asarm_ins_cond (const cs_arm * arm, unsigned int ins_n) {
    struct list * list = list_create();
    switch (arm->cc) {
    /***************************************************************************
    * ARM_CC_EQ
    ***************************************************************************/
    case ARM_CC_EQ : {
        list_append_(list, bins_ce_(boper_variable(1, "Z"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_NE
    ***************************************************************************/
    case ARM_CC_NE : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_Z"),
                                     boper_variable(1, "Z"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_ce_(boper_variable(1, "NOT_Z"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_HS
    ***************************************************************************/
    case ARM_CC_HS : {
        list_append_(list, bins_ce_(boper_variable(1, "C"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_LO
    ***************************************************************************/
    case ARM_CC_LO : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_C"),
                                     boper_variable(1, "C"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_ce_(boper_variable(1, "NOT_C"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_MI
    ***************************************************************************/
    case ARM_CC_MI : {
        list_append_(list, bins_ce_(boper_variable(1, "N"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_PL
    ***************************************************************************/
    case ARM_CC_PL : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_N"),
                                     boper_variable(1, "N"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_ce_(boper_variable(1, "NOT_N"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_VS
    ***************************************************************************/
    case ARM_CC_VS : {
        list_append_(list, bins_ce_(boper_variable(1, "V"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_VC
    ***************************************************************************/
    case ARM_CC_VC : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_V"),
                                     boper_variable(1, "V"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_ce_(boper_variable(1, "NOT_V"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_HI
    ***************************************************************************/
    case ARM_CC_HI : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_Z"),
                                     boper_variable(1, "Z"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_and_(boper_variable(1, "C_AND_NOT_Z"),
                                     boper_variable(1, "NOT_Z"),
                                     boper_variable(1, "C")));
        list_append_(list, bins_ce_(boper_variable(1, "C_AND_NOT_Z"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_LS
    ***************************************************************************/
    case ARM_CC_LS : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_C"),
                                     boper_variable(1, "C"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_or_(boper_variable(1, "NOT_C_OR_Z"),
                                    boper_variable(1, "NOT_C"),
                                    boper_variable(1, "Z")));
        list_append_(list, bins_ce_(boper_variable(1, "NOT_C_OR_Z"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_GE
    ***************************************************************************/
    case ARM_CC_GE : {
        list_append_(list, bins_cmpeq_(boper_variable(1, "N_EQ_V"),
                                       boper_variable(1, "N"),
                                       boper_variable(1, "V")));
        list_append_(list, bins_ce_(boper_variable(1, "N_EQ_V"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_LT
    ***************************************************************************/
    case ARM_CC_LT : {
        list_append_(list, bins_cmpeq_(boper_variable(1, "N_EQ_V"),
                                      boper_variable(1, "N"),
                                      boper_variable(1, "V")));
        list_append_(list, bins_xor_(boper_variable(1, "N_NEQ_V"),
                                     boper_variable(1, "N_EQ_V"),
                                     boper_constant(1, 1)));
        list_append(list, bins_ce_(boper_variable(1, "N_NEQ_V"),
                                   boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_GT
    ***************************************************************************/
    case ARM_CC_GT : {
        list_append_(list, bins_xor_(boper_variable(1, "NOT_Z"),
                                     boper_variable(1, "Z"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_cmpeq_(boper_variable(1, "N_EQ_V"),
                                       boper_variable(1, "N"),
                                       boper_variable(1, "V")));
        list_append_(list, bins_and_(boper_variable(1, "NOT_Z_AND_N_EQ_V"),
                                     boper_variable(1, "NOT_Z"),
                                     boper_variable(1, "N_EQ_V")));
        list_append_(list, bins_ce_(boper_variable(1, "NOT_Z_AND_N_EQ_V"),
                                    boper_constant(8, ins_n)));
        break;
    }
    /***************************************************************************
    * ARM_CC_LE
    ***************************************************************************/
    case ARM_CC_LE :
        list_append_(list, bins_cmpeq_(boper_variable(1, "N_EQ_V"),
                                      boper_variable(1, "N"),
                                      boper_variable(1, "V")));
        list_append_(list, bins_xor_(boper_variable(1, "N_NEQ_V"),
                                     boper_variable(1, "N_EQ_V"),
                                     boper_constant(1, 1)));
        list_append_(list, bins_and_(boper_variable(1, "Z_AND_N_NEQ_V"),
                                     boper_variable(1, "Z"),
                                     boper_variable(1, "N_NEQ_V")));
        list_append_(list, bins_ce_(boper_variable(1, "Z_AND_N_NEQ_V"),
                                    boper_constant(8, ins_n)));
        break;
    /***************************************************************************
    * ARM_CC_AL
    ***************************************************************************/
    case ARM_CC_AL :
        break;
    /***************************************************************************
    * ARM_CC_INVALID
    ***************************************************************************/
    case ARM_CC_INVALID :
        btlog_error("encountered ARM_CC_INVALID as an instruction condition");
        break;
    }

    return list;
}


struct boper * asarm_operand (struct list * list, cs_arm_op * op) {
    switch (op->type) {
    case ARM_OP_REG : {
        struct boper * boper = asarm_cs_reg(op->reg);
        if (boper == NULL)
            return NULL;

        switch (op->shift.type) {
        /***********************************************************************
        * ARM_SFT_ASR
        ***********************************************************************/
        case ARM_SFT_ASR :
            /* if shift_imm == 0 */
            if (op->shift.value == 0) {
                list_append_(list, bins_shr_(
                    boper_variable(32, "shifter_carry_out32"),
                    OCOPY(boper),
                    boper_constant(32, 31)
                ));
                list_append_(list, bins_trun_(
                    boper_variable(1, "shifter_carry_out"),
                    boper_variable(32, "shifter_carry_out32")
                ));
                list_append_(list, bins_or_(
                    boper_variable(32, "shifter_operand"),
                    boper_constant(32, 0),
                    boper_constant(32, 0)
                ));
                list_append_(list, bins_ce_(
                    boper_variable(1, "shifter_carry_out"),
                    boper_constant(8, 1)
                ));
                list_append_(list, bins_or_(
                    boper_variable(32, "shifter_operand"),
                    boper_constant(32, 0xffffffff),
                    boper_constant(32, 0xffffffff)
                ));
            }
            /* if shift_imm > 0 */
            else {
                list_append_(list, bins_shr_(
                    boper_variable(32, "shifter_carry_out32"),
                    OCOPY(boper),
                    boper_constant(32, op->shift.value - 1)
                ));
                list_append_(list, bins_trun_(
                    boper_variable(1, "shifter_carry_out"),
                    boper_variable(32, "shifter_carry_out32")
                ));
                struct list * asr_list = bins_asr_(
                    boper_variable(32, "shifter_operand"),
                    boper,
                    boper_constant(32, op->shift.value)
                );
                list_append_list(list, asr_list);
                ODEL(asr_list);
            }
            return boper_variable(32, "shifter_operand");
        /***********************************************************************
        * ARM_SFT_LSL
        ***********************************************************************/
        case ARM_SFT_LSL :
            if (op->shift.value == 0) {
                list_append_(list, bins_or_(
                    boper_variable(1, "shifter_carry_out"),
                    boper_variable(1, "C"),
                    boper_variable(1, "C")
                ));
                return boper;
            }
            else {
                list_append_(list, bins_shl_(
                    boper_variable(32, "shifter_operand"),
                    OCOPY(boper),
                    boper_constant(32, op->shift.value)
                ));
                list_append_(list, bins_shr_(
                    boper_variable(32, "shifter_carry_out32"),
                    boper,
                    boper_constant(32, op->shift.value)
                ));
                list_append_(list, bins_trun_(
                    boper_variable(1, "shifter_carry_out"),
                    boper_variable(32, "shifter_carry_out32")
                ));
                return boper_variable(32, "shifter_operand");
            }
        /***********************************************************************
        * ARM_SFT_LSR
        ***********************************************************************/
        case ARM_SFT_LSR :
            if (op->shift.value == 0) {
                list_append_(list, bins_or_(
                    boper_variable(32, "shifter_operand"),
                    boper_constant(32, 0),
                    boper_constant(32, 0)
                ));
                list_append_(list, bins_shr_(
                    boper_variable(32, "shifter_carry_out32"),
                    boper,
                    boper_constant(32, 31)
                ));
            }
            else {
                list_append_(list, bins_shr_(
                    boper_variable(32, "shifter_operand"),
                    OCOPY(boper),
                    boper_constant(32, op->shift.value)
                ));
                list_append_(list, bins_shr_(
                    boper_variable(32, "shifter_carry_out32"),
                    boper,
                    boper_constant(32, op->shift.value - 1)
                ));
            }
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));
            return boper_variable(32, "shifter_operand");
        /***********************************************************************
        * ARM_SFT_ROR
        ***********************************************************************/
        case ARM_SFT_ROR :
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                OCOPY(boper),
                boper_constant(32, op->shift.value - 1)
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));
            struct list * ror_ins = bins_ror_(
                boper_variable(32, "shifter_operand"),
                boper,
                boper_constant(32, op->shift.value)
            );
            list_append_list(list, ror_ins);
            ODEL(ror_ins);
            return boper_variable(32, "shifter_operand");
        /***********************************************************************
        * ARM_SFT_RRX
        ***********************************************************************/
        case ARM_SFT_RRX :
            list_append_(list, bins_shr_(boper_variable(32, "rrx_shr"),
                                         OCOPY(boper),
                                         boper_constant(32, 1)));
            list_append_(list, bins_zext_(boper_variable(32, "C32"),
                                          boper_variable(1, "C")));
            list_append_(list, bins_shl_(boper_variable(32, "rrx_shl"),
                                         boper_variable(32, "C32"),
                                         boper_constant(32, 31)));
            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_variable(32, "rrx_shl"),
                                        boper_variable(32, "rrx_shr")));
            list_append_(list, bins_and_(
                boper_variable(32, "shifter_carry_out32"),
                boper,
                boper_constant(32, 1)
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));
            return boper_variable(32, "shifter_operand");
        /***********************************************************************
        * ARM_SFT_ASR_REG
        ***********************************************************************/
        case ARM_SFT_ASR_REG : {
            struct boper * rs = asarm_cs_reg(op->shift.value);

            list_append_(list, bins_and_(boper_variable(32, "operand_rs"),
                                         rs,
                                         boper_constant(32, 0xff)));

            /* operand_rs == 0 */
            list_append_(list, bins_cmpeq(boper_variable(1, "operand_rs_EQ_0"),
                                          boper_variable(32, "operand_rs"),
                                          boper_constant(32, 0)));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_EQ_0"),
                                        boper_constant(8, 2)));
            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 0)));
            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_variable(1, "C"),
                                        boper_variable(1, "C")));

            /* operand_rs < 32 */
            list_append_(list, bins_cmpltu_(
                boper_variable(1, "operand_rs_LTU_32"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 32)
            ));

            struct list * asr_list = bins_asr_(
                boper_variable(32, "shifter_operand"),
                OCOPY(boper),
                boper_variable(32, "operand_rs")
            );
            list_append_(list, bins_ce_(
                boper_variable(1, "operand_rs_LTU_32"),
                boper_constant(8, list_length(asr_list) + 3)
            ));
            list_append_list(list, asr_list);
            ODEL(asr_list);

            list_append_(list, bins_sub_(
                boper_variable(32, "shifter_carry_out_bit"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 1)
            ));
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                boper_variable(32, "operand_rs"),
                boper_variable(32, "shifter_carry_out_bit")
            ));
            list_append_(list, bins_trun_(
                boper_variable(32, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));

            /* operand_rs >= 32 */
            list_append_(list, bins_cmpltu_(
                boper_variable(1, "operand_rs_GEU_32"),
                boper_constant(32, 31),
                boper_variable(32, "operand_rs")
            ));
            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_GEU_32"),
                                        boper_constant(8, 6)));

            list_append_(list, bins_shr_(
                boper_variable(1, "operand_rm_31_32"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 31)
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "operand_rm_31"),
                boper_variable(32, "operand_rm_31_32")
            ));

            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_variable(1, "operand_rm_31"),
                                        boper_variable(1, "operand_rm_31")));
            /* rm[31] == 0 */
            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 0)));
            /* rm[31] == 1 */
            list_append_(list, bins_ce_(boper_variable(1, "operand_rm_31"),
                                        boper_constant(8, 1)));
            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0xffffffff),
                                        boper_constant(32, 0xffffffff)));

            ODEL(boper);
            return boper_variable(32, "shifter_operand");
        }
        /***********************************************************************
        * ARM_SFT_LSL_REG
        ***********************************************************************/
        case ARM_SFT_LSL_REG : {
            struct boper * rs = asarm_cs_reg(op->shift.value);

            list_append_(list, bins_and_(boper_variable(32, "operand_rs"),
                                         rs,
                                         boper_constant(32, 0xff)));
            /* rm = 0 */
            list_append_(list, bins_cmpeq_(
                boper_variable(1, "operand_rs_EQ_0"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 0)
            ));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_EQ_0"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        OCOPY(boper),
                                        OCOPY(boper)));
            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_variable(1, "C"),
                                        boper_variable(1, "C")));

            /* rm < 32 */
            list_append_(list, bins_cmpltu_(
                boper_variable(1, "operand_rs_LTU_32"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 32)
            ));

            list_append_(list, bins_ce_(boper_variable(32, "operand_rs_LTU_32"),
                                        boper_constant(8, 4)));

            list_append_(list, bins_shl_(boper_variable(32, "shifter_operand"),
                                         OCOPY(boper),
                                         boper_variable(32, "operand_rs")));
            list_append_(list, bins_sub_(
                boper_variable(32, "shifter_carry_out_bit"),
                boper_constant(32, 32),
                boper_variable(32, "operand_rs")
            ));
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                OCOPY(boper),
                boper_variable(32, "shifter_carry_out_bit")
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));

            /* rm == 32 */
            list_append_(list, bins_cmpeq_(
                boper_variable(1, "operand_rs_EQ_32"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 0)
            ));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_EQ_32"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 0)));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                OCOPY(boper)
            ));

            /* rm > 32 */
            list_append_(list, bins_cmpltu_(
                boper_variable(1, "operand_rs_GTU_32"),
                boper_constant(32, 32),
                boper_variable(32, "operand_rs")
            ));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_GTU_32"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 0)));
            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_constant(1, 0),
                                        boper_constant(1, 0)));

            ODEL(boper);

            return boper_variable(32, "shifter_operand");
        }
        /***********************************************************************
        * ARM_SFT_LSR_REG
        ***********************************************************************/
        /* this is near-identical to ARM_SFT_LSL_REG */
        case ARM_SFT_LSR_REG : {
            struct boper * rs = asarm_cs_reg(op->shift.value);

            list_append_(list, bins_and_(boper_variable(32, "operand_rs"),
                                         rs,
                                         boper_constant(32, 0xff)));
            /* rs = 0 */
            list_append_(list, bins_cmpeq_(
                boper_variable(1, "operand_rs_EQ_0"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 0)
            ));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_EQ_0"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        OCOPY(boper),
                                        OCOPY(boper)));
            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_variable(1, "C"),
                                        boper_variable(1, "C")));

            /* rs < 32 */
            list_append_(list, bins_cmpltu_(
                boper_variable(1, "operand_rs_LTU_32"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 32)
            ));

            list_append_(list, bins_ce_(boper_variable(32, "operand_rs_LTU_32"),
                                        boper_constant(8, 4)));

            list_append_(list, bins_shr_(boper_variable(32, "shifter_operand"),
                                         OCOPY(boper),
                                         boper_variable(32, "operand_rs")));
            list_append_(list, bins_sub_(
                boper_variable(32, "shifter_carry_out_bit"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 1)
            ));
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                OCOPY(boper),
                boper_variable(32, "shifter_carry_out_bit")
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));

            /* rs == 32 */
            list_append_(list, bins_cmpeq_(
                boper_variable(1, "operand_rs_EQ_32"),
                boper_variable(32, "operand_rs"),
                boper_constant(32, 0)
            ));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_EQ_32"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 0)));
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                OCOPY(boper),
                boper_constant(32, 31)
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));

            /* rs > 32 */
            list_append_(list, bins_cmpltu_(
                boper_variable(1, "operand_rs_GTU_32"),
                boper_constant(32, 32),
                boper_variable(32, "operand_rs")
            ));

            list_append_(list, bins_ce_(boper_variable(1, "operand_rs_GTU_32"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        boper_constant(32, 0),
                                        boper_constant(32, 0)));
            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_constant(1, 0),
                                        boper_constant(1, 0)));

            ODEL(boper);

            return boper_variable(32, "shifter_operand");
        }
        /***********************************************************************
        * ARM_SFT_ROR_REG
        ***********************************************************************/
        case ARM_SFT_ROR_REG : {
            struct boper * rs = asarm_cs_reg(op->shift.value);

            list_append_(list, bins_and_(boper_variable(32, "operand_rs"),
                                         rs,
                                         boper_constant(32, 0xff)));

            list_append_(list, bins_cmpeq_(boper_variable(1, "rs_EQ_0"),
                                           boper_variable(32, "operand_rs"),
                                           boper_constant(32, 0)));

            list_append_(list, bins_ce_(boper_variable(1, "rs_EQ_0"),
                                        boper_constant(8, 2)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        OCOPY(boper),
                                        OCOPY(boper)));
            list_append_(list, bins_or_(boper_variable(1, "shifter_carry_out"),
                                        boper_variable(1, "C"),
                                        boper_variable(1, "C")));

            list_append_(list, bins_and_(boper_variable(32, "rs_1f"),
                                         boper_variable(32, "operand_rs"),
                                         boper_constant(32, 0x1f)));
            list_append_(list, bins_cmpeq_(boper_variable(1, "rs_1f_EQ_0"),
                                           boper_variable(32, "rs_1f"),
                                           boper_constant(32, 0)));

            list_append_(list, bins_ce_(boper_variable(1, "rs_1f_EQ_0"),
                                        boper_constant(8, 3)));

            list_append_(list, bins_or_(boper_variable(32, "shifter_operand"),
                                        OCOPY(boper),
                                        OCOPY(boper)));
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                OCOPY(boper),
                boper_constant(32, 31)
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));

            list_append_(list, bins_cmpltu_(boper_variable(1, "rs_1f_GT_0"),
                                            boper_constant(32, 0),
                                            boper_variable(32, "rs_1f")));
            
            struct list * ror_list = bins_ror(
                boper_variable(32, "shifter_operand"),
                OCOPY(boper),
                boper_variable(32, "rs_1f")
            );
            list_append_(list, bins_ce_(
                boper_variable(1, "rs_1f_GT_0"),
                boper_constant(8, list_length(ror_list) + 3)
            ));
            ODEL(ror_list);
            list_append_(list, bins_sub_(
                boper_variable(32, "shifter_carry_out_bit"),
                boper_variable(32, "rs_1f_GT_0"),
                boper_constant(32, 1)
            ));
            list_append_(list, bins_shr_(
                boper_variable(32, "shifter_carry_out32"),
                OCOPY(boper),
                boper_variable(32, "shifter_carry_out_bit")
            ));
            list_append_(list, bins_trun_(
                boper_variable(1, "shifter_carry_out"),
                boper_variable(32, "shifter_carry_out32")
            ));

            return boper_variable(32, "shifter_operand");
        }
        /***********************************************************************
        * ARM_SFT_RRX_REG
        ***********************************************************************/
        case ARM_SFT_RRX_REG :
            btlog_error("UNHANDLED ARM_SFT_RRX_REG");
            break;
        /***********************************************************************
        * ARM_SFT_INVALID
        ***********************************************************************/
        case ARM_SFT_INVALID :
            return boper;
        }
        break;
    }
    case ARM_OP_IMM :
        btlog_error("UNHANDLED ARM_OP_IMM");
        break;
    case ARM_OP_MEM :
        btlog_error("UNHANDLED ARM_OP_MEM");
        break;
    case ARM_OP_FP :
        btlog_error("UNHANDLED ARM_OP_FP");
        break;
    case ARM_OP_CIMM :
        btlog_error("UNHANDLED ARM_OP_CIMM");
        break;
    case ARM_OP_PIMM :
        btlog_error("UNHANDLED ARM_OP_PIMM");
        break;
    case ARM_OP_SETEND :
        btlog_error("UNHANDLED ARM_OP_SETEND");
        break;
    case ARM_OP_SYSREG :
        btlog_error("UNHANDLED ARM_OP_SYSREG");
        break;
    case ARM_OP_INVALID :
        btlog_error("ENCOUNTERED ARM_OP_INVALID");
        break;
    }
    return NULL;
}


struct list * arm_translate_ins (
    const void * buf,
    size_t size,
    uint64_t address
) {
    csh handle;
    cs_insn * insn;
    size_t count;

    if (cs_open(CS_ARCH_ARM, CS_MODE_THUMB, &handle) != CS_ERR_OK)
        return NULL;
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

    count = cs_disasm(handle, buf, size, address, 1, &insn);
    if (count < 1) {
        cs_close(&handle);
        return NULL;
    }

    cs_insn * ins = &(insn[0]);
    cs_arm * arm = &(ins->detail->arm);

    struct list * list = list_create();

    switch (ins->id) {

    /***************************************************************************
    * ARM_INS_ADD
    ***************************************************************************/
    case ARM_INS_ADD : {
        struct list * add_ins_list = list_create();
        struct boper * rd = asarm_operand(add_ins_list, &(arm->operands[0]));
        struct boper * rn = asarm_operand(add_ins_list, &(arm->operands[1]));
        struct boper * shifter_operand;
        shifter_operand = asarm_operand(add_ins_list, &(arm->operands[2]));

        if ((rd == NULL) || (rn == NULL) || (shifter_operand == NULL)) {
            btlog_error("ARM_INS_ADD an operand was null");
            return NULL;
        }

        list_append_(add_ins_list, bins_add_(OCOPY(rd),
                                             OCOPY(rn),
                                             OCOPY(shifter_operand)));

        if (arm->update_flags) {
            if (arm->operands[0].reg == ARM_REG_R15)
                list_append_(add_ins_list, bins_or_(
                    boper_variable(32, "CPSR"),
                    boper_variable(32, "SPSR"),
                    boper_variable(32, "SPSR")
                ));

            list_append_(add_ins_list, bins_cmplts_(boper_variable(1, "N"),
                                                    OCOPY(rd),
                                                    boper_constant(32, 0)));
            list_append_(add_ins_list, bins_cmpeq_(boper_variable(1, "Z"),
                                                   OCOPY(rd),
                                                   boper_constant(32, 0)));
            list_append_(add_ins_list, bins_cmpltu_(boper_variable(1, "C"),
                                                    OCOPY(rd),
                                                    OCOPY(rn)));
            list_append_(add_ins_list, bins_cmples_(
                boper_variable(1, "VxorO"),
                OCOPY(rn),
                OCOPY(shifter_operand)
            ));
            list_append_(add_ins_list, bins_xor_(boper_variable(1, "V"),
                                                 boper_variable(1, "VxorO"),
                                                 boper_variable(1, "O")));
        }

        struct list * cond_list = asarm_ins_cond(arm, list_length(add_ins_list));
        list_append_list(list, cond_list);
        list_append_list(list, add_ins_list);
        list_append_(list, bins_add(boper_variable(32, "pc"),
                                    boper_variable(32, "pc"),
                                    boper_constant(32, ins->size)));
        ODEL(cond_list);
        ODEL(add_ins_list);
        break;
    }
        
    /***************************************************************************
    * UNHANDLED INSTRUCTION
    ***************************************************************************/
    default : {
        char error_buf[128];
        unsigned int i;
        for (i = 0; i < ins->size; i++)
            sprintf(&(error_buf[i * 2]), "%02X", ins->bytes[i]);
        btlog_error("UNHANDLED INSTRUCTION %s %s %s",
                    error_buf, ins->mnemonic, ins->op_str);
        break;
    }
    }

    if (list_length(list) > 0)

    cs_free(insn, count);
    cs_close(&handle);

    return list;
}


struct list * arm_translate_block (
    const void * buf,
    size_t size,
    uint64_t address
) {
    return NULL;
}