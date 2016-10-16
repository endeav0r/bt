#ifndef amd64_HEADER
#define amd64_HEADER

#include <stdint.h>

#include "arch/arch.h"
#include "bt/bins.h"
#include "container/byte_buf.h"
#include "container/list.h"
#include "container/varstore.h"

extern const struct arch_target arch_target_amd64;


struct byte_buf * amd64_assemble (struct list * btins_list,
                                  struct varstore * varstore);


/*
Return codes:
0 - Execution Successful
1 - Error reading from MMU
2 - Error writing to MMO
3 - Encountered HLT instruction
*/
unsigned int amd64_execute (const void * code,
                            struct varstore * varstore);

int amd64_load_r_boper (struct byte_buf * bb,
                        struct varstore * varstore,
                        unsigned int reg,
                        struct boper * boper);

int amd64_store_boper_r (struct byte_buf * bb,
                         struct varstore * varstore,
                         struct boper * boper,
                         unsigned int reg);

int amd64_store_boper_imm (struct byte_buf * bb,
                           struct varstore * varstore,
                           struct boper * boper,
                           uint64_t imm);

int add_r_imm (struct byte_buf * bb,
               unsigned int dst,
               uint64_t imm,
               unsigned int bits);

int add_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits);

int add_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int and_r_imm (struct byte_buf * bb,
               unsigned int dst,
               uint64_t imm,
               unsigned int bits);

int and_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits);

int and_rm_imm (struct byte_buf * bb,
                unsigned int rm,
                uint32_t off32,
                uint64_t imm,
                unsigned int bits);

int and_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int call_r (struct byte_buf * bb, unsigned int r);

int cmp_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits);

int cmp_r_r (struct byte_buf * bb,
             unsigned int lhs,
             unsigned int rhs,
             unsigned int bits);

int div_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int jcc (struct byte_buf * bb, unsigned int condition, int offset);

int jmp (struct byte_buf * bb, int offset);

int mod_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int mov_r_imm (struct byte_buf * bb,
               unsigned int r,
               uint64_t imm,
               unsigned int bits);

int mov_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits);

int mov_r_rm (struct byte_buf * bb,
              unsigned int r,
              unsigned int rm,
              uint32_t off32,
              unsigned int bits);

int mov_rm_imm (struct byte_buf * bb,
                unsigned int rm,
                uint32_t off32,
                uint64_t imm,
                unsigned int bits);

int mov_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int mov_r64_r64 (struct byte_buf * bb, unsigned int dst, unsigned int rhs);

int movsx_r_r (struct byte_buf * bb,
               unsigned int dst,
               unsigned int dst_bits,
               unsigned int src,
               unsigned int src_bits);

int movzx_r_r (struct byte_buf * bb,
               unsigned int dst,
               unsigned int dst_bits,
               unsigned int src,
               unsigned int src_bits);

int mul_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int or_rm_r (struct byte_buf * bb,
             unsigned int rm,
             uint32_t off32,
             unsigned int r,
             unsigned int bits);

int pop_r64 (struct byte_buf * bb, unsigned int reg);

int push_r64 (struct byte_buf * bb, unsigned int reg);

int ret (struct byte_buf * bb);

int shl_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int shr_r64_r64 (struct byte_buf * bb, unsigned int lhs, unsigned int rhs);

int sub_r_imm (struct byte_buf * bb,
               unsigned int dst,
               uint64_t imm,
               unsigned int bits);

int sub_r_r (struct byte_buf * bb,
             unsigned int dst,
             unsigned int rhs,
             unsigned int bits);

int sub_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

int xor_rm_r (struct byte_buf * bb,
              unsigned int rm,
              uint32_t off32,
              unsigned int r,
              unsigned int bits);

#endif