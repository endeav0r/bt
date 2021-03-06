#include "arch/source/hsvm.h"
#include "arch/target/amd64.h"
#include "btlog.h"
#include "bt/bins.h"
#include "bt/jit.h"
#include "container/byte_buf.h"
#include "container/list.h"
#include "container/memmap.h"
#include "container/varstore.h"
#include "hooks.h"
#include "platform/hsvm.h"
#include "plugins/plugins.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "plugins/tainttrace.c"


uint16_t get_rip (struct varstore * varstore) {
    size_t offset;
    int error = varstore_offset(varstore, "rip", 16, &offset);
    if (error) {
        fprintf(stderr, "failed to get rip from varstore\n");
        return -1;
    }
    uint8_t * data_buf = (uint8_t *) varstore_data_buf(varstore);
    uint16_t rip = *((uint16_t *) &(data_buf[offset]));

    return rip;
}


int main (int argc, char * argv[]) {
    /* turn on debugging */
    btlog_continuous("jit_hsvm.debug");

    /* initialize global hooks */
    global_hooks_init();

    /* Start up plugins */
    struct plugins * plugins = plugins_create();

    /* Test out tainttracer plugin. */
    //if (plugins_load(plugins, "plugins/tainttrace.so"))
    //    fprintf(stderr, "Failed to load tainttrace\n");
    plugin_initialize();
    global_hooks_append(plugin_hooks());

    /* Read in HSVM binary */
    FILE * fh = fopen(argv[1], "rb");
    if (fh == NULL) {
        fprintf(stderr, "could not open file %s\n", argv[1]);
        return -1;
    }

    fseek(fh, 0, SEEK_END);
    size_t filesize = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    uint8_t * buf = malloc(filesize);
    if (fread(buf, 1, filesize, fh) != filesize) {
        fprintf(stderr, "failed to read %s\n", argv[1]);
        return -1;
    }

    fclose(fh);

    btlog("[jit_hsvm] read %s %u bytes", argv[1], (unsigned int) filesize);

    /* Create the memmap */
    struct memmap * memmap = memmap_create(4096);

    btlog("[jit_hsvm] created memmap");
    fflush(stdout);

    /* insert our code into memmap */
    memmap_map(memmap,
               0,
               0x10000,
               buf,
               filesize,
               MEMMAP_R | MEMMAP_W | MEMMAP_X);

    free(buf);

    btlog("[jit_hsvm] initialized memmap");
    fflush(stdout);

    /* create our jit */
    struct jit * jit = jit_create(&arch_source_hsvm,
                                  &arch_target_amd64,
                                  &platform_hsvm);
    btlog("[jit_hsvm] created jit");
    fflush(stdout);

    /* create our varstore */
    struct varstore * varstore = varstore_create();
    btlog("[jit_hsvm] created varstore");
    fflush(stdout);

    /* init and set rip */
    size_t offset = varstore_insert(varstore, "rip", 16);
    uint8_t * data_buf = (uint8_t *) varstore_data_buf(varstore);
    *((uint16_t *) &(data_buf[offset])) = 0;
    btlog("[jit_hsvm] rip set and init");

    /* init and set rsp */
    offset = varstore_insert(varstore, "rsp", 16);
    data_buf = (uint8_t *) varstore_data_buf(varstore);
    *((uint16_t *) &(data_buf[offset])) = 0xfff8;
    btlog("[jit_hsvm] rsp set and init");

    /* call our global hooks for jit startup */
    global_hooks_call(HOOK_JIT_STARTUP, jit, varstore, memmap);

    int result = jit_execute(jit, varstore, memmap);
    fprintf(stderr, "jit result %d\n", result);

    /* call our global hooks for jit cleanup */
    global_hooks_call(HOOK_JIT_CLEANUP, jit, varstore, memmap);

    /* clean up */
    ODEL(varstore);
    ODEL(jit);
    ODEL(memmap);
    global_hooks_cleanup();
    btlog("[jit_hsvm] calling plugins_delete");
    plugins_delete(plugins);
    plugin_cleanup();

    return 0;
}
