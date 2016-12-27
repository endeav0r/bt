#ifndef asx86_HEADER
#define asx86_HEADER

#include "arch/arch.h"
#include "container/list.h"

extern const struct arch_source arch_source_x86;

const char *  asx86_ip_variable_identifier ();
unsigned int  asx86_ip_variable_bits ();
struct list * asx86_translate_ins   (const void * buf, size_t size);
struct list * asx86_translate_block (const void * buf, size_t size);

uint32_t asx86_buf_to_uint32 (const uint8_t * buf);
uint16_t asx86_buf_to_uint16 (const uint8_t * buf);

int asx86_store_le32_ (struct list * list,
                       struct boper * address,
                       struct boper * value);

int asx86_store_le16_ (struct list * list,
                      struct boper * address,
                      struct boper * value);

struct boper * asx86_reg_full_to_8 (struct list * list, unsigned int reg);

struct boper * asx86_reg_full_to_16 (struct list * list, unsigned int reg);

int asx86_set_8bit_reg (struct list * list,
                        unsigned int reg,
                        struct boper * value);

int asx86_set_16bit_reg (struct list * list,
                         unsigned int reg,
                         struct boper * value);

struct boper * asx86_sib (struct list * list,
                          const uint8_t * bytes,
                          size_t bytes_size,
                          unsigned int bits);

struct boper * asx86_modrm_ea (struct list * list,
                               const uint8_t * bytes,
                               size_t bytes_size,
                               unsigned int bits);

#endif
