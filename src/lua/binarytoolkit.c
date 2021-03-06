#include "binarytoolkit.h"

#include <stdint.h>
#include <stdio.h>

#include "arch/source/hsvm.h"
#include "arch/source/arm.h"
#include "arch/target/amd64.h"
#include "bt/bins.h"
#include "bt/jit.h"
#include "container/buf.h"
#include "container/list.h"
#include "container/memmap.h"
#include "platform/hsvm.h"


static const struct luaL_Reg lbt_lib_f [] = {
    {"arch_source_arm", lbt_arch_source_arm},
    {"arch_source_hsvm", lbt_arch_source_hsvm},

    {"boper_constant", lbt_boper_constant},
    {"boper_variable", lbt_boper_variable},
    {"buf",            lbt_buf_new},
    {"jit",            lbt_jit_new},
    {"memmap",         lbt_memmap_new},
    {"varstore",       lbt_varstore_new},

    {NULL, NULL}
};


static const struct luaL_Reg lbt_arch_source_m [] = {
    {"__gc", lbt_arch_source_gc},
    {"ip_variable_identifier", lbt_arch_source_ip_variable_identifier},
    {"ip_variable_bits", lbt_arch_source_ip_variable_bits},
    {"translate_ins", lbt_arch_source_translate_ins},
    {"translate_block", lbt_arch_source_translate_block},
    {NULL, NULL}
};


static const struct luaL_Reg lbt_boper_m [] = {
    {"__gc", lbt_boper_gc},
    {"type", lbt_boper_type},
    {"identifier", lbt_boper_identifier},
    {"bits", lbt_boper_bits},
    {"value", lbt_boper_value},
    {NULL, NULL}
};


static const struct luaL_Reg lbt_bins_m [] = {
    {"__gc", lbt_bins_gc},
    {"string", lbt_bins_string},
    {"__tostring", lbt_bins_string},
    {NULL, NULL}
};



static const struct luaL_Reg lbt_buf_m [] = {
    {"__gc", lbt_buf_gc},
    {"length", lbt_buf_length},
    {"slice", lbt_buf_slice},
    {"get", lbt_buf_get},
    {NULL, NULL}
};


static const struct luaL_Reg lbt_jit_m [] = {
    {"__gc", lbt_jit_gc},
    {NULL, NULL}
};


static const struct luaL_Reg lbt_memmap_m [] = {
    {"__gc",    lbt_memmap_gc},
    {"map",     lbt_memmap_map},
    {"get_buf", lbt_memmap_get_buf},
    {"u8",      lbt_memmap_get_u8},
    {NULL, NULL}
};


static const struct luaL_Reg lbt_varstore_m [] = {
    {"__gc", lbt_varstore_gc},
    {NULL, NULL}
};


struct lbt_constant {
    const char * name;
    uint64_t value;
};


enum {
    LBT_ARCH_SOURCE_ARM,
    LBT_ARCH_SOURCE_HSVM,
    LBT_ARCH_TARGET_AMD64,
    LBT_PLATFORM_HSVM
};


struct lbt_constant lbt_constants [] = {
    {"MEMMAP_R", .value=1},
    {"MEMMAP_W", .value=2},
    {"MEMMAP_X", .value=4},
    {"MEMMAP_NOFAIL", .value=1},
    {"ARCH_SOURCE_ARM", .value=LBT_ARCH_SOURCE_ARM},
    {"ARCH_SOURCE_HSVM", .value=LBT_ARCH_SOURCE_HSVM},
    {"ARCH_TARGET_AMD64", .value=LBT_ARCH_TARGET_AMD64},
    {"PLATFORM_HSVM", .value=LBT_PLATFORM_HSVM},
    {"BOP_ADD", BOP_ADD},
    {"BOP_SUB", BOP_SUB},
    {"BOP_UMUL", BOP_UMUL},
    {"BOP_UMOD", BOP_UMOD},
    {"BOP_AND", BOP_AND},
    {"BOP_OR", BOP_OR},
    {"BOP_XOR", BOP_XOR},
    {"BOP_SHL", BOP_SHL},
    {"BOP_SHR", BOP_SHR},
    {"BOP_CMPEQ", BOP_CMPEQ},
    {"BOP_CMPLTU", BOP_CMPLTU},
    {"BOP_CMPLTS", BOP_CMPLTS},
    {"BOP_CMPLEU", BOP_CMPLEU},
    {"BOP_CMPLES", BOP_CMPLES},
    {"BOP_SEXT", BOP_SEXT},
    {"BOP_ZEXT", BOP_ZEXT},
    {"BOP_TRUN", BOP_TRUN},
    {"BOP_STORE", BOP_STORE},
    {"BOP_LOAD", BOP_LOAD},
    {"BOP_CE", BOP_CE},
    {"BOP_HLT", BOP_HLT},
    {"BOP_COMMENT", BOP_COMMENT},
    {"BOP_HOOK", BOP_HOOK},
    {NULL, .value=-1}
};


LUALIB_API int luaopen_binarytoolkit (lua_State * L) {
    luaL_newmetatable(L, "lbt.arch_source");
    luaL_setfuncs(L, lbt_arch_source_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.bins");
    luaL_setfuncs(L, lbt_bins_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.boper");
    luaL_setfuncs(L, lbt_boper_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.buf");
    luaL_setfuncs(L, lbt_buf_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.jit");
    luaL_setfuncs(L, lbt_jit_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.memmap");
    luaL_setfuncs(L, lbt_memmap_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.varstore");
    luaL_setfuncs(L, lbt_varstore_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newlib(L, lbt_lib_f);

    unsigned int i;
    for (i = 0; lbt_constants[i].name != NULL; i++) {
        lua_pushstring(L, lbt_constants[i].name);
        lua_pushinteger(L, lbt_constants[i].value);
        lua_settable(L, -3);
    }

    return 1;
}






/*******************************************************************************
* arch_source
*******************************************************************************/
int lbt_arch_source_push (
    lua_State * L,
    const struct arch_source * arch_source_to_push
) {
    const struct arch_source ** arch_source = lua_newuserdata(L, sizeof(struct buf *));
    luaL_getmetatable(L, "lbt.arch_source");
    lua_setmetatable(L, -2);

    *arch_source = arch_source_to_push;

    return 1;
}


const struct arch_source * lbt_arch_source_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.arch_source");
    luaL_argcheck(L, userdata != NULL, position, "lbt.arch_source expected");
    return (const struct arch_source *) *userdata;
}


int lbt_arch_source_gc (lua_State * L) {
    return 0;
}


int lbt_arch_source_arm (lua_State * L) {
    return lbt_arch_source_push(L, &arch_source_arm);
}


int lbt_arch_source_hsvm (lua_State * L) {
    return lbt_arch_source_push(L, &arch_source_hsvm);
}


int lbt_arch_source_ip_variable_identifier (lua_State * L) {
    const struct arch_source * arch_source = lbt_arch_source_check(L, 1);

    lua_pushstring(L, arch_source->ip_variable_identifier());

    return 1;
}


int lbt_arch_source_ip_variable_bits (lua_State * L) {
    const struct arch_source * arch_source = lbt_arch_source_check(L, 1);

    lua_pushinteger(L, arch_source->ip_variable_bits());

    return 1;
}


int lbt_arch_source_translate_ins (lua_State * L) {
    const struct arch_source * arch_source = lbt_arch_source_check(L, 1);
    const void * buf = luaL_checkstring(L, 2);
    lua_len(L, 2);
    size_t size = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    uint64_t address = luaL_checkinteger(L, 3);

    struct list * list = arch_source->translate_ins(buf, size, address);

    if (list == NULL)
        luaL_error(L, "error in arch_source->translate_ins");

    lua_newtable(L);

    unsigned int i = 1;
    struct list_it * it;
    for (it = list_it(list); it != NULL; it = list_it_next(it)) {
        struct bins * bins = list_it_data(it);

        lua_pushinteger(L, i++);
        lbt_bins_absorb(L, OCOPY(bins));
        lua_settable(L, -3);
    }

    ODEL(list);
    /* TODO push all the bins here */
    return 1;
}


int lbt_arch_source_translate_block (lua_State * L) {
    const struct arch_source * arch_source = lbt_arch_source_check(L, 1);
    const void * buf = luaL_checkstring(L, 2);
    lua_len(L, 2);
    size_t size = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    uint64_t address = luaL_checkinteger(L, 3);

    struct list * list = arch_source->translate_block(buf, size, address);

    if (list == NULL)
        luaL_error(L, "error in arch_source->translate_block");

    lua_newtable(L);

    unsigned int i = 1;
    struct list_it * it;
    for (it = list_it(list); it != NULL; it = list_it_next(it)) {
        struct bins * bins = list_it_data(it);

        lua_pushinteger(L, i++);
        lbt_bins_absorb(L, OCOPY(bins));
        lua_settable(L, -3);
    }

    ODEL(list);
    /* TODO push all the bins here */
    return 1;
}



/*******************************************************************************
* bins
*******************************************************************************/
struct bins * lbt_bins_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.bins");
    luaL_argcheck(L, userdata != NULL, position, "lbt.bins expected");
    return (struct bins *) *userdata;
}


int lbt_bins_absorb (lua_State * L, struct bins * bins_to_absorb) {
    struct bins ** bins = lua_newuserdata(L, sizeof(struct bins *));
    luaL_getmetatable(L, "lbt.bins");
    lua_setmetatable(L, -2);

    *bins = bins_to_absorb;

    return 1;
}


int lbt_bins_gc (lua_State * L) {
    struct bins * bins = lbt_bins_check(L, 1);
    ODEL(bins);
    return 0;
}


int lbt_bins_string (lua_State * L) {
    struct bins * bins = lbt_bins_check(L, 1);

    char * s = bins_string(bins);

    lua_pushstring(L, s);

    free(s);

    return 1;
}


/*******************************************************************************
* boper
*******************************************************************************/
struct boper * lbt_boper_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.boper");
    luaL_argcheck(L, userdata != NULL, position, "lbt.boper expected");
    return (struct boper *) *userdata;
}


int lbt_boper_absorb (lua_State * L, struct boper * boper_to_absorb) {
    struct boper ** boper = lua_newuserdata(L, sizeof(struct boper *));
    luaL_getmetatable(L, "lbt.boper");
    lua_setmetatable(L, -2);

    *boper = boper_to_absorb;

    return 1;
}


int lbt_boper_gc (lua_State * L) {
    struct boper * boper = lbt_boper_check(L, 1);
    ODEL(boper);
    return 0;
}


int lbt_boper_constant (lua_State * L) {
    uint64_t bits = luaL_checkinteger(L, 1);
    uint64_t value = luaL_checkinteger(L, 2);

    return lbt_boper_absorb(L, boper_constant(bits, value));
}


int lbt_boper_variable (lua_State * L) {
    uint64_t bits = luaL_checkinteger(L, 1);
    const char * identifier = luaL_checkstring(L, 2);

    return lbt_boper_absorb(L, boper_variable(bits, identifier));
}


int lbt_boper_type (lua_State * L) {
    struct boper * boper = lbt_boper_check(L, 1);

    if (boper_type(boper) == BOPER_VARIABLE)
        lua_pushstring(L, "variable");
    else if (boper_type(boper) == BOPER_CONSTANT)
        lua_pushstring(L, "contant");
    else
        luaL_error(L, "error on lua_boper_type, invalid return result");

    return 1;
}


int lbt_boper_identifier (lua_State * L) {
    struct boper * boper = lbt_boper_check(L, 1);

    lua_pushstring(L, boper_identifier(boper));

    return 1;
}


int lbt_boper_bits (lua_State * L) {
    struct boper * boper = lbt_boper_check(L, 1);

    lua_pushinteger(L, boper_bits(boper));

    return 1;
}


int lbt_boper_value (lua_State * L) {
    struct boper * boper = lbt_boper_check(L, 1);

    if (boper_type(boper) != BOPER_CONSTANT)
        luaL_error(L, "boper_value called on non-constant boper");

    lua_pushinteger(L, boper_value(boper));

    return 1;
}



/*******************************************************************************
* buf
*******************************************************************************/
int lbt_buf_absorb (lua_State * L, struct buf * buf_to_absorb) {
    struct buf ** buf = lua_newuserdata(L, sizeof(struct buf *));
    luaL_getmetatable(L, "lbt.buf");
    lua_setmetatable(L, -2);

    *buf = buf_to_absorb;

    return 1;
}


struct buf * lbt_buf_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.buf");
    luaL_argcheck(L, userdata != NULL, position, "lbt.buf expected");
    return (struct buf *) *userdata;
}


int lbt_buf_new (lua_State * L) {
    uint64_t length = luaL_checkinteger(L, 1);

    struct buf ** buf = lua_newuserdata(L, sizeof(struct buf *));
    luaL_getmetatable(L, "lbt.buf");
    lua_setmetatable(L, -2);

    *buf = buf_create(length);

    return 1;
}


int lbt_buf_gc (lua_State * L) {
    struct buf * buf = lbt_buf_check(L, 1);
    ODEL(buf);
    return 0;
}


int lbt_buf_length (lua_State * L) {
    struct buf * buf = lbt_buf_check(L, 1);

    lua_pushinteger(L, buf_length(buf));

    return 1;
}


int lbt_buf_slice (lua_State * L) {
    struct buf * buf = lbt_buf_check(L, 1);
    size_t offset = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);

    struct buf * slice = buf_slice(buf, offset, size);

    if (slice == NULL)
        luaL_error(L, "failed to get slice from buf");

    lbt_buf_absorb(L, slice);

    return 1;
}


int lbt_buf_get (lua_State * L) {
    struct buf * buf = lbt_buf_check(L, 1);
    size_t offset = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);

    const void * data = buf_get(buf, offset, size);
    if (data == NULL)
        luaL_error(L, "failed to get data from buf");

    lua_pushlstring(L, data, size);

    return 1;
}



/*******************************************************************************
* jit
*******************************************************************************/


struct jit * lbt_jit_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.jit");
    luaL_argcheck(L, userdata != NULL, position, "lbt.jit expected");
    return (struct jit *) *userdata;
}


int lbt_jit_new (lua_State * L) {
    unsigned int lbt_source = luaL_checkinteger(L, 1);
    unsigned int lbt_target = luaL_checkinteger(L, 2);
    unsigned int lbt_platform = luaL_checkinteger(L, 3);

    const struct arch_source * arch_source = NULL;
    const struct arch_target * arch_target = NULL;
    const struct platform * platform = NULL;

    if (lbt_source == LBT_ARCH_SOURCE_HSVM)
        arch_source = &arch_source_hsvm;
    else if (lbt_source == LBT_ARCH_SOURCE_ARM)
        arch_source = &arch_source_arm;

    if (lbt_target == LBT_ARCH_TARGET_AMD64)
        arch_target = &arch_target_amd64;

    if (lbt_platform == LBT_PLATFORM_HSVM)
        platform = &platform_hsvm;

    if (arch_source == NULL)
        luaL_error(L, "bad arch source");
    else if (arch_target == NULL)
        luaL_error(L, "bad arch target");
    else if (platform == NULL)
        luaL_error(L, "bad platform");

    struct jit ** jit = lua_newuserdata(L, sizeof(struct jit *));
    luaL_getmetatable(L, "lbt.jit");
    lua_setmetatable(L, -2);

    *jit = jit_create(arch_source, arch_target, platform);

    return 1;
}


int lbt_jit_gc (lua_State * L) {
    struct jit * jit = lbt_jit_check(L, 1);
    ODEL(jit);
    return 0;
}


int lbt_jit_execute (lua_State * L) {
    struct jit * jit = lbt_jit_check(L, 1);
    struct varstore * varstore = lbt_varstore_check(L, 2);
    struct memmap * memmap = lbt_memmap_check(L, 3);

    int result = jit_execute(jit, varstore, memmap);

    lua_pushinteger(L, result);

    return 1;
}


/*******************************************************************************
* memmap
*******************************************************************************/


struct memmap * lbt_memmap_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.memmap");
    luaL_argcheck(L, userdata != NULL, position, "lbt.memmap expected");
    return (struct memmap *) *userdata;
}


int lbt_memmap_new (lua_State * L) {
    uint64_t page_size = luaL_checkinteger(L, 0);

    struct memmap ** memmap = lua_newuserdata(L, sizeof(struct memmap *));
    luaL_getmetatable(L, "lbt.memmap");
    lua_setmetatable(L, -2);

    *memmap = memmap_create(page_size);

    return 1;
}


int lbt_memmap_gc (lua_State * L) {
    struct memmap * memmap = lbt_memmap_check(L, 1);
    ODEL(memmap);
    return 0;
}


int lbt_memmap_map (lua_State * L) {
    struct memmap * memmap = lbt_memmap_check(L, 1);
    uint64_t address = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);
    const void * buf = luaL_checkstring(L, 4);
    lua_len(L, 4);
    size_t buf_size = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    unsigned int permissions = luaL_checkinteger(L, 5);

    int error = memmap_map(memmap, address, size, buf, buf_size, permissions);
    if (error)
        luaL_error(L, "error in memmap_map");

    return 0;
}


int lbt_memmap_get_buf (lua_State * L) {
    struct memmap * memmap = lbt_memmap_check(L, 1);
    uint64_t address = luaL_checkinteger(L, 2);
    size_t size = luaL_checkinteger(L, 3);

    struct buf * buf = memmap_get_buf(memmap, address, size);

    if (buf == NULL)
        luaL_error(L, "error getting buf from memmap");

    lbt_buf_absorb(L, buf);

    return 1;
}


int lbt_memmap_get_u8 (lua_State * L) {
    struct memmap * memmap = lbt_memmap_check(L, 1);
    uint64_t address = luaL_checkinteger(L, 2);

    uint8_t u8;
    if (memmap_get_u8(memmap, address, &u8))
        luaL_error(L, "error getting u8 from memmap");

    lua_pushinteger(L, u8);

    return 1;
}



/*******************************************************************************
* varstore
*******************************************************************************/


struct varstore * lbt_varstore_check (lua_State * L, int position) {
    void ** userdata = (void **) luaL_checkudata(L, position, "lbt.varstore");
    luaL_argcheck(L, userdata != NULL, position, "lbt.varstore expected");
    return (struct varstore *) *userdata;
}


int lbt_varstore_new (lua_State * L) {
    uint64_t length = luaL_checkinteger(L, 0);

    struct varstore ** varstore = lua_newuserdata(L, sizeof(struct varstore *));
    luaL_getmetatable(L, "lbt.varstore");
    lua_setmetatable(L, -2);

    *varstore = varstore_create(length);

    return 1;
}


int lbt_varstore_gc (lua_State * L) {
    struct varstore * varstore = lbt_varstore_check(L, 1);
    ODEL(varstore);
    return 0;
}