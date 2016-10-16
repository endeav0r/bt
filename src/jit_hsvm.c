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


struct reg_code_table {
    unsigned int reg_code;
    const char * reg_string;
};


struct reg_code_table reg_codes [] = {
    {0x0, "r0"},
    {0x1, "r1"},
    {0x2, "r2"},
    {0x3, "r3"},
    {0x4, "r4"},
    {0x5, "r5"},
    {0x6, "r6"},
    {0xA, "r7"},
    {0x8, "rbp"},
    {0x9, "rsp"},
    {0x7, "rip"},
    {-1, NULL}
};


const char * get_reg_string (unsigned int reg_code) {
    unsigned int i;
    for (i = 0; reg_codes[i].reg_string != NULL; i++) {
        if (reg_codes[i].reg_code == reg_code)
            return reg_codes[i].reg_string;
    }
    return NULL;
}


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
               0x10000,
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

    // init and set rsp
    offset = varstore_insert(varstore, "rsp", 16);
    data_buf = (uint8_t *) varstore_data_buf(varstore);
    *((uint16_t *) &(data_buf[offset])) = 0xfff8;
    printf("rsp set and init\n");
    fflush(stdout);

    //unsigned int hlt_code = 0;

    do {
        uint16_t rip = get_rip(varstore);
        //if (rip == 0x38e) return 0;
        //#define DEBUG
        #ifdef DEBUG
        printf("rip 0x%04x\n", rip);fflush(stdout);
        #endif
        int result = jit_execute(jit, varstore, memmap);
        //if (rip == 0x37a) break;
        if (result < 0) {
            fprintf(stderr, "jit_execute error %d rip=%04x\n", result, rip);
            return -1;
        }
        else if (result == 3) {
            int error = varstore_offset(varstore, "halt_code", 8, &offset);
            // OUT reg
            if (data_buf[offset] == 2) {
                varstore_offset(varstore, "out_reg", 8, &offset);
                unsigned int reg_code = data_buf[offset];
                varstore_offset(varstore, get_reg_string(reg_code), 16, &offset);
                uint8_t r = *((uint16_t *) &(data_buf[offset]));
                write(0, &r, 1);
            }
            // IN reg
            else if (data_buf[offset] == 1) {
                varstore_offset(varstore, "in_reg", 8, &offset);
                unsigned int reg_code = data_buf[offset];
                offset = varstore_offset_create(varstore, get_reg_string(reg_code), 16);
                uint8_t r;
                read(1, &r, 1);
                *((uint16_t *) &(data_buf[offset])) = r;
            }
            else {
                printf("halt code %d\n", data_buf[offset]);
                break;
            }
        }
        else if (result) {
            fprintf(stderr, "jit result %d\n", result);
            break;
        }
    } while(1);

    ODEL(varstore);
    ODEL(jit);
    ODEL(memmap);

    return 0;
}