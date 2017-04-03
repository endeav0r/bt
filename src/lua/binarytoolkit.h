#ifndef lua_HEADER
#define lua_HEADER

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


LUALIB_API int luaopen_binarytoolkit (lua_State * L);


const struct arch_source * lbt_arch_source_check (lua_State * L, int position);
int lbt_arch_source_push (
    lua_State * L,
    const struct arch_source * arch_source
);
int lbt_arch_source_gc  (lua_State * L);

int lbt_arch_source_hsvm (lua_State * L);
int lbt_arch_source_arm  (lua_State * L);

int lbt_arch_source_ip_variable_identifier (lua_State * L);
int lbt_arch_source_ip_variable_bits       (lua_State * L);
int lbt_arch_source_translate_ins          (lua_State * L);
int lbt_arch_source_translate_block        (lua_State * L);


struct bins * lbt_bins_check (lua_State * L, int position);
int lbt_bins_absorb (lua_State * L, struct bins * bins_to_absorb);
int lbt_bins_gc     (lua_State * L);
int lbt_bins_string (lua_State * L);


struct boper * lbt_boper_check (lua_State * L, int position);
int lbt_boper_absorb   (lua_State * L, struct boper * boper_to_absorb);
int lbt_boper_constant (lua_State * L);
int lbt_boper_variable (lua_State * L);
int lbt_boper_gc       (lua_State * L);
int lbt_boper_type     (lua_State * L);
int lbt_boper_identifier (lua_State * L);
int lbt_boper_bits     (lua_State * L);
int lbt_boper_value    (lua_State * L);


struct buf * lbt_buf_check (lua_State * L, int position);
int lbt_buf_absorb (lua_State * L, struct buf * buf_to_absorb);
int lbt_buf_gc     (lua_State * L);
int lbt_buf_new    (lua_State * L);
int lbt_buf_length (lua_State * L);
int lbt_buf_slice  (lua_State * L);
int lbt_buf_get    (lua_State * L);


struct jit * lbt_jit_check (lua_State * L, int position);
int lbt_jit_new     (lua_State * L);
int lbt_jit_gc      (lua_State * L);
int lbt_jit_execute (lua_State * L);


struct memmap * lbt_memmap_check (lua_State * L, int position);
int lbt_memmap_new        (lua_State * L);
int lbt_memmap_gc         (lua_State * L);
int lbt_memmap_map        (lua_State * L);
int lbt_memmap_get_buf    (lua_State * L);
int lbt_memmap_get_u8     (lua_State * L);
/*
int lbt_memmap_get_u16_le (lua_State * L);
int lbt_memmap_get_u16_be (lua_State * L);
int lbt_memmap_get_u32_le (lua_State * L);
int lbt_memmap_get_u32_be (lua_State * L);
int lbt_memmap_get_u64_le (lua_State * L);
int lbt_memmap_get_u64_be (lua_State * L);
int lbt_memmap_set_u8     (lua_State * L);
int lbt_memmap_set_u16_le (lua_State * L);
int lbt_memmap_set_u16_be (lua_State * L);
int lbt_memmap_set_u32_le (lua_State * L);
int lbt_memmap_set_u32_be (lua_State * L);
int lbt_memmap_set_u64_le (lua_State * L);
int lbt_memmap_set_u64_be (lua_State * L);
*/

struct varstore * lbt_varstore_check (lua_State * L, int position);
int lbt_varstore_new (lua_State * L);
int lbt_varstore_gc  (lua_State * L);

#endif