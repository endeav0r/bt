#include "arch/source/hsvm.h"
#include "arch/target/amd64.h"
#include "bt/bins.h"
#include "bt/jit.h"
#include "container/byte_buf.h"
#include "container/list.h"
#include "container/memmap.h"
#include "container/varstore.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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
    // Read in HSVM binary
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

    printf("read %s %u bytes\n", argv[1], (unsigned int) filesize);
    fflush(stdout);

    // Create the memmap
    struct memmap * memmap = memmap_create(4096);

    printf("created memmap\n");
    fflush(stdout);

    // insert our code into memmap
    memmap_map(memmap,
               0,
               filesize,
               buf,
               filesize,
               MEMMAP_R | MEMMAP_W | MEMMAP_X);

    printf("initialized memmap\n");
    fflush(stdout);

    // create our jit
    struct jit * jit = jit_create(&arch_source_hsvm, &arch_target_amd64);
    printf("created jit\n");
    fflush(stdout);

    // create our varstore
    struct varstore * varstore = varstore_create();
    printf("created varstore\n");
    fflush(stdout);

    // init and set rip
    size_t offset = varstore_insert(varstore, "rip", 16);
    uint8_t * data_buf = (uint8_t *) varstore_data_buf(varstore);
    *((uint16_t *) &(data_buf[offset])) = 0;
    printf("rip set and init\n");
    fflush(stdout);

    //unsigned int hlt_code = 0;

    do {
        // get instruction pointer
        int error = varstore_offset(varstore, "rip", 16, &offset);
        if (error) {
            fprintf(stderr, "failed to get rip from varstore\n");
            return -1;
        }
        uint8_t * data_buf = (uint8_t *) varstore_data_buf(varstore);
        uint16_t rip = *((uint16_t *) &(data_buf[offset]));

        printf("read rip, rip is %u\n", rip);
        fflush(stdout);

        // get memory pointed to by rip
        struct buf * buf = memmap_get_buf(memmap, rip, 256);
        /* TODO CHECK LENGTH OF RETURNED BUF */

        printf("memmap_get_buf returned %p length=%u\n",
                buf,
                (unsigned int) buf_length(buf));
        fflush(stdout);

        // translate instructions
        struct list * list;
        list = arch_source_hsvm.translate_block(buf_get(buf, 0, buf_length(buf)),
                                                buf_length(buf));

        struct list_it * it;
        for (it = list_it(list); it != NULL; it = list_it_next(it)) {
            struct bins * bins = list_it_obj(it);
            char * s = bins_string(bins);
            printf("%s\n", s);
            free(s);
        }

        // assemble code
        struct byte_buf * assembled_buf;
        assembled_buf = arch_target_amd64.assemble(list, varstore);

        if (assembled_buf == NULL) {
            fprintf(stderr, "failed to assemble instructions\n");
            return -1;
        }

        // insert into jit
        jit_set_code(jit,
                     rip,
                     byte_buf_bytes(assembled_buf),
                     byte_buf_length(assembled_buf));


        // welcome to the fucking rodeo
        const void * code_ptr = jit_get_code(jit, rip);
        if (code_ptr == NULL) {
            fprintf(stderr, "failed to get code ptr for code at %04x\n", rip);
            return -1;
        }

        unsigned int ret_code = arch_target_amd64.execute(code_ptr, varstore);

        printf("ret_code %u\n", ret_code);

        printf("rip %04x\n", get_rip(varstore));


        ODEL(buf);
        ODEL(list);
    } while(0);

    ODEL(varstore);
    ODEL(jit);
    ODEL(memmap);

    return 0;
}