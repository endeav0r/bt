#include "binarytoolkit.h"

#include <stdint.h>
#include <stdio.h>

#include "arch/source/hsvm.h"
#include "arch/target/amd64.h"
#include "container/buf.h"
#include "container/memmap.h"
#include "platform/hsvm.h"


static const struct luaL_Reg lbt_lib_f [] = {
    {"test", lbt_test},

    {"buf",    lbt_buf_new},
    {"memmap", lbt_memmap_new},

    {NULL, NULL}
};


static const struct luaL_Reg lbt_buf_m [] = {
    {"__gc", lbt_buf_gc},
    {"length", lbt_buf_length},
    {"slice", lbt_buf_slice},
    {"get", lbt_buf_get},
    {NULL, NULL}
};


static const struct luaL_Reg lbt_memmap_m [] = {
    {"__gc",    lbt_memmap_gc},
    {"map",     lbt_memmap_map},
    {"get_buf", lbt_memmap_get_buf},
    {"u8",      lbt_memmap_get_u8},
    {NULL, NULL}
};


struct lbt_constant {
    const char * name;
    union {
        uint64_t value;
        const void * ptr;
    };
};


struct lbt_constant lbt_constants [] = {
    {"MEMMAP_R", .value=1},
    {"MEMMAP_W", .value=2},
    {"MEMMAP_X", .value=4},
    {"MEMMAP_NOFAIL", .value=1},
    {"arch_source_hsvm", .ptr=&arch_source_hsvm},
    {"arch_target_amd64", .ptr=&arch_target_amd64},
    {"platform_hsvm", .ptr=&platform_hsvm},
    {NULL, .value=-1}
};


LUALIB_API int luaopen_binarytoolkit (lua_State * L) {
    luaL_newmetatable(L, "lbt.buf");
    luaL_setfuncs(L, lbt_buf_m, 0);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_newmetatable(L, "lbt.memmap");
    luaL_setfuncs(L, lbt_memmap_m, 0);
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



int lbt_test (lua_State * L) {
    printf("this is a test\n");
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
    uint64_t length = luaL_checkinteger(L, 0);

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