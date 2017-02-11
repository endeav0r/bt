#include "asx86_decode.h"


/*
* The following flags come from the opcode column in the instruction summary
* table, and tell us how to interpret the opcodes.
* These are placed in the opcode_column field in struct asx86_decode_ins.
*/

/* There are no operands for this instruction */
#define DECODE_NONE 0

/*
* A /digit between 0 and 7 indicates that the ModR/M of the instruction
* uses only the R/M of the operand. The reg field contains a digit that
* provides an extension to the instruction's opcode.
* We use the first bit to determine whether or not this field is present, and
* then the next three bits to determine the value of that field.
*/
#define DECODE_DIGIT (1 << 1)
#define DECODE_0     ((0 << 2) | DECODE_DIGIT)
#define DECODE_1     ((1 << 2) | DECODE_DIGIT)
#define DECODE_2     ((2 << 2) | DECODE_DIGIT)
#define DECODE_3     ((3 << 2) | DECODE_DIGIT)
#define DECODE_4     ((4 << 2) | DECODE_DIGIT)
#define DECODE_5     ((5 << 2) | DECODE_DIGIT)
#define DECODE_6     ((6 << 2) | DECODE_DIGIT)
#define DECODE_7     ((7 << 2) | DECODE_DIGIT)
#define DECODE_DIGIT_MASK (7 << 2)

/* Indicates that the ModR/M byte of the instruction contains a register
   operand and an r/m operand. */
#define DECODE_R (1 << 5)

/* Same as DECODE_R, but specifies RM is first/dst operand, and R is second/src
   operand. */
#define DECODE_RM_R ((1 << 6) | DECODE_R)

/* Same as DECODE_R, but specified R is first/dst operand, and RM is second/src
   operand. */
#define DECODE_R_RM ((1 << 7) | DECODE_R)

/* There is a ModR/M byte, but we only care about the RM portion. */
#define DECODE_RM (1 << 8)

/* A 1-byte (IB), 2-byte (IW), or 4-byte (ID) immediate operand to the
   instruction that follows the opcode, ModR/M or SIB bytes. */
#define DECODE_IB (1 << 9)
#define DECODE_IW (1 << 10)
#define DECODE_ID (1 << 11)

/* A 1-byte (CB), 2-byte (CW), or 4-byte (CD) value following the opcode. The
   value is used to specify a code offset and possible a new value for the code
   segment register. */
#define DECODE_CB (1 << 12)
#define DECODE_CW (1 << 13)
#define DECODE_CD (1 << 14)

struct asx86_decode_table {
    uint8_t byte0;
    enum asx86_decode_opcode_precise opcode_precise;
    enum asx86_decode_opcode opcode;
    unsigned int opcode_column;
};

struct asx86_decode_table asx86_decode_table[] = {
    {0x00, X86P_ADD_RM8_R8,          X86_ADD,  DECODE_RM_R},
    {0x01, X86P_ADD_RM32_R32,        X86_ADD,  DECODE_RM_R},
    {0x02, X86P_ADD_R8_RM8,          X86_ADD,  DECODE_R_RM},
    {0x03, X86P_ADD_R32_RM32,        X86_ADD,  DECODE_R_RM},
    {0x04, X86P_ADD_AL_IMM8,         X86_ADD,  DECODE_IB},
    {0x05, X86P_ADD_EAX_IMM32,       X86_ADD,  DECODE_ID},
    {0x10, X86P_ADC_RM8_R8,          X86_ADC,  DECODE_RM_R},
    {0x11, X86P_ADC_RM32_R32,        X86_ADC,  DECODE_RM_R},
    {0x12, X86P_ADC_R8_RM8,          X86_ADC,  DECODE_R_RM},
    {0x13, X86P_ADC_R32_RM32,        X86_ADC,  DECODE_R_RM},
    {0x14, X86P_ADC_AL_IMM8,         X86_ADC,  DECODE_IB},
    {0x15, X86P_ADC_EAX_IMM32,       X86_ADC,  DECODE_ID},
    {0x20, X86P_AND_RM8_R8,          X86_AND,  DECODE_RM_R},
    {0x21, X86P_AND_RM32_R32,        X86_AND,  DECODE_RM_R},
    {0x22, X86P_AND_R8_RM8,          X86_AND,  DECODE_R_RM},
    {0x23, X86P_AND_R32_RM32,        X86_AND,  DECODE_R_RM},
    {0x24, X86P_AND_AL_IMM8,         X86_AND,  DECODE_IB},
    {0x24, X86P_AND_EAX_IMM32,       X86_AND,  DECODE_ID},
    {0x38, X86P_CMP_RM8_R8,          X86_CMP,  DECODE_RM_R},
    {0x39, X86P_CMP_RM32_R32,        X86_CMP,  DECODE_RM_R},
    {0x3A, X86P_CMP_R8_RM8,          X86_CMP,  DECODE_RM_R},
    {0x3B, X86P_CMP_R32_RM32,        X86_CMP,  DECODE_RM_R},
    {0x3C, X86P_CMP_AL_IMM8,         X86_CMP,  DECODE_IB},
    {0x3D, X86P_CMP_EAX_IMM32,       X86_CMP,  DECODE_ID},
    {0x40, X86P_INC_EAX,             X86_INC,  DECODE_NONE},
    {0x41, X86P_INC_ECX,             X86_INC,  DECODE_NONE},
    {0x42, X86P_INC_EDX,             X86_INC,  DECODE_NONE},
    {0x43, X86P_INC_EBX,             X86_INC,  DECODE_NONE},
    {0x44, X86P_INC_ESP,             X86_INC,  DECODE_NONE},
    {0x45, X86P_INC_EBP,             X86_INC,  DECODE_NONE},
    {0x46, X86P_INC_ESI,             X86_INC,  DECODE_NONE},
    {0x47, X86P_INC_EDI,             X86_INC,  DECODE_NONE},
    {0x48, X86P_DEC_EAX,             X86_DEC,  DECODE_NONE},
    {0x49, X86P_DEC_ECX,             X86_DEC,  DECODE_NONE},
    {0x4A, X86P_DEC_EDX,             X86_DEC,  DECODE_NONE},
    {0x4B, X86P_DEC_EBX,             X86_DEC,  DECODE_NONE},
    {0x4C, X86P_DEC_ESP,             X86_DEC,  DECODE_NONE},
    {0x4D, X86P_DEC_EBP,             X86_DEC,  DECODE_NONE},
    {0x4E, X86P_DEC_ESI,             X86_DEC,  DECODE_NONE},
    {0x4F, X86P_DEC_EDI,             X86_DEC,  DECODE_NONE},
    {0x69, X86P_IMUL_R32_RM32_IMM32, X86_IMUL, DECODE_R_RM | DECODE_ID},
    {0x6B, X86P_IMUL_R32_RM32_IMM8,  X86_IMUL, DECODE_R_RM | DECODE_IB},
    {0x70, X86P_JO,                  X86_JO,   DECODE_CB},
    {0x71, X86P_JNO,                 X86_JNO,  DECODE_CB},
    {0x72, X86P_JB,                  X86_JB,   DECODE_CB},
    {0x73, X86P_JAE,                 X86_JAE,  DECODE_CB},
    {0x74, X86P_JE,                  X86_JE,   DECODE_CB},
    {0x75, X86P_JNE,                 X86_JNE,  DECODE_CB},
    {0x76, X86P_JBE,                 X86_JBE,  DECODE_CB},
    {0x77, X86P_JA,                  X86_JA,   DECODE_CB},
    {0x78, X86P_JS,                  X86_JS,   DECODE_CB},
    {0x79, X86P_JNS,                 X86_JNS,  DECODE_CB},
    {0x7A, X86P_JP,                  X86_JP,   DECODE_CB},
    {0x7C, X86P_JL,                  X86_JL,   DECODE_CB},
    {0x7D, X86P_JGE,                 X86_JGE,  DECODE_CB},
    {0x7E, X86P_JLE,                 X86_JLE,  DECODE_CB},
    {0x7F, X86P_JG,                  X86_JG,   DECODE_CB},
    {0x80, X86P_ADD_RM8_IMM8,        X86_ADD,  DECODE_0 | DECODE_RM | DECODE_IB},
    {0x80, X86P_ADC_RM8_IMM8,        X86_ADC,  DECODE_2 | DECODE_RM | DECODE_IB},
    {0x80, X86P_AND_RM8_IMM8,        X86_AND,  DECODE_4 | DECODE_RM | DECODE_IB},
    {0x80, X86P_CMP_RM8_IMM8,        X86_CMP,  DECODE_7 | DECODE_RM | DECODE_IB},
    {0x81, X86P_ADD_RM32_IMM32,      X86_ADD,  DECODE_0 | DECODE_RM | DECODE_ID},
    {0x81, X86P_ADC_RM32_IMM32,      X86_ADC,  DECODE_2 | DECODE_RM | DECODE_ID},
    {0x81, X86P_AND_RM32_IMM32,      X86_AND,  DECODE_4 | DECODE_RM | DECODE_ID},
    {0x81, X86P_CMP_RM32_IMM32,      X86_CMP,  DECODE_7 | DECODE_RM | DECODE_ID},
    {0x83, X86P_ADD_RM32_IMM8,       X86_ADD,  DECODE_0 | DECODE_RM | DECODE_IB},
    {0x83, X86P_ADC_RM32_IMM8,       X86_ADC,  DECODE_2 | DECODE_RM | DECODE_IB},
    {0x83, X86P_AND_RM32_IMM8,       X86_AND,  DECODE_4 | DECODE_RM | DECODE_IB},
    {0x83, X86P_CMP_RM32_IMM8,       X86_CMP,  DECODE_7 | DECODE_RM | DECODE_IB},
    {0x98, X86P_CWDE,                X86_CWDE, DECODE_NONE},
    {0xE8, X86P_CALL_REL32,          X86_CALL, DECODE_CD},
    {0xF6, X86P_IMUL_RM8,            X86_IMUL, DECODE_5 | DECODE_RM},
    {0xF6, X86P_DIV_RM8,             X86_DIV,  DECODE_6 | DECODE_RM},
    {0xF6, X86P_IDIV_RM8,            X86_IDIV, DECODE_7 | DECODE_RM},
    {0xF7, X86P_IMUL_RM32,           X86_IMUL, DECODE_5 | DECODE_RM},
    {0xF7, X86P_DIV_RM32,            X86_DIV,  DECODE_6 | DECODE_RM},
    {0xF7, X86P_IDIV_RM32,           X86_IDIV, DECODE_7 | DECODE_RM},
    {0xF8, X86P_CLC,                 X86_CLC,  DECODE_NONE},
    {0xFC, X86P_CLD,                 X86_CLD,  DECODE_NONE},
    {0xF8, X86P_INC_RM8,             X86_INC,  DECODE_0 | DECODE_RM},
    {0xFE, X86P_DEC_RM8,             X86_DEC,  DECODE_1 | DECODE_RM},
    {0xFF, X86P_INC_RM32,            X86_INC,  DECODE_0 | DECODE_RM},
    {0xFF, X86P_DEC_RM32,            X86_DEC,  DECODE_1 | DECODE_RM},
    {0xFF, X86P_CALL_RM32,           X86_CALL, DECODE_2 | DECODE_RM}
};
