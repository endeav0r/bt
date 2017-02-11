#ifndef asx86_decode_HEADER
#define asx86_decode_HEADER

#include <inttypes.h>

enum asx86_decode_operand_type {
    REL8, /* Relative signed 8-bit offset from end of instruction */
    REL16, /* Relative signed 16-bit offset from end of instruction */
    REL32, /* Relative signed 32-bit offset from end of instruction */
    R8,  /* One of the byte GPRs, al, bl, etc. */
    R16, /* One of the word GPRs, ax, bx, etc. */
    R32, /* One of the doubleword GPRs, eax, ebx, etc. */
    /* An immediate byte value. Sign-extended when combined with word or
       doubleword */
    IMM8,
    /* An immediate word value. Sign-extended when combined with a doubleword */
    IMM16,
    /* An immediate doubleword value. */
    IMM32,
    RM8,  /* ModR/M byte value */
    RM16, /* ModR/M word value */
    RM32, /* ModR/M doubleword value */
};

/* This is our enum of supported opcodes for each decoding we support. */
enum asx86_decode_opcode_precise {
    X86P_ADC_AL_IMM8,
    X86P_ADC_EAX_IMM32,
    X86P_ADC_RM8_IMM8,
    X86P_ADC_RM32_IMM32,
    X86P_ADC_RM32_IMM8,
    X86P_ADC_RM8_R8,
    X86P_ADC_RM32_R32,
    X86P_ADC_R8_RM8,
    X86P_ADC_R32_RM32,
    X86P_ADD_AL_IMM8,
    X86P_ADD_EAX_IMM32,
    X86P_ADD_RM8_IMM8,
    X86P_ADD_RM32_IMM32,
    X86P_ADD_RM32_IMM8,
    X86P_ADD_RM8_R8,
    X86P_ADD_RM32_R32,
    X86P_ADD_R8_RM8,
    X86P_ADD_R32_RM32,
    X86P_AND_AL_IMM8,
    X86P_AND_EAX_IMM32,
    X86P_AND_RM8_IMM8,
    X86P_AND_RM32_IMM32,
    X86P_AND_RM32_IMM8,
    X86P_AND_R8_RM8,
    X86P_AND_R32_RM32,
    X86P_AND_RM8_R8,
    X86P_AND_RM32_R32,
    X86P_CALL_REL32,
    X86P_CALL_RM32,
    X86P_CWDE,
    X86P_CLC,
    X86P_CLD,
    X86P_CMP_AL_IMM8,
    X86P_CMP_EAX_IMM32,
    X86P_CMP_RM8_IMM8,
    X86P_CMP_RM32_IMM32,
    X86P_CMP_RM32_IMM8,
    X86P_CMP_RM8_R8,
    X86P_CMP_RM32_R32,
    X86P_CMP_R8_RM8,
    X86P_CMP_R32_RM32,
    X86P_DEC_EAX,
    X86P_DEC_ECX,
    X86P_DEC_EDX,
    X86P_DEC_EBX,
    X86P_DEC_ESP,
    X86P_DEC_EBP,
    X86P_DEC_ESI,
    X86P_DEC_EDI,
    X86P_DEC_RM8,
    X86P_DEC_RM32,
    X86P_DIV_RM8,
    X86P_DIV_RM32,
    X86P_IDIV_RM8,
    X86P_IDIV_RM32,
    X86P_IMUL_RM8,
    X86P_IMUL_RM32,
    X86P_IMUL_R32_RM32_IMM8,
    X86P_IMUL_R32_RM32_IMM32,
    X86P_INC_RM8,
    X86P_INC_RM32,
    X86P_INC_EAX,
    X86P_INC_ECX,
    X86P_INC_EDX,
    X86P_INC_EBX,
    X86P_INC_ESP,
    X86P_INC_EBP,
    X86P_INC_ESI,
    X86P_INC_EDI,
    X86P_JA,
    X86P_JAE,
    X86P_JB,
    X86P_JBE,
    X86P_JE,
    X86P_JG,
    X86P_JGE,
    X86P_JL,
    X86P_JLE,
    X86P_JNE,
    X86P_JNO,
    X86P_JNS,
    X86P_JO,
    X86P_JP,
    X86P_JS,
};

enum asx86_decode_opcode {
    X86_ADC,
    X86_ADD,
    X86_AND,
    X86_CALL,
    X86_CBW,
    X86_CLC,
    X86_CLD,
    X86_CWDE,
    X86_CMP,
    X86_DEC,
    X86_DIV,
    X86_IDIV,
    X86_IMUL,
    X86_INC,
    X86_JA,
    X86_JAE,
    X86_JB,
    X86_JBE,
    X86_JE,
    X86_JG,
    X86_JGE,
    X86_JL,
    X86_JLE,
    X86_JNE,
    X86_JNO,
    X86_JNS,
    X86_JO,
    X86_JP,
    X86_JS,
};

struct asx86_decode_operand {
    enum asx86_decode_operand_type type;
    union {
        uint8_t reg;
        uint8_t modrm;
        int32_t imm;
    };
    uint8_t sib;
    unsigned int bit_width;
};

struct asx86_decode_ins {
    enum asx86_decode_opcode opcode;
    struct asx86_decode_operand operands[3];
};

#endif
