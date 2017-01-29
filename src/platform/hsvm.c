#include "platform/hsvm.h"

#include "arch/hsvm.h"
#include "btlog.h"
#include "container/varstore.h"

#include <unistd.h>


const struct platform platform_hsvm = {
    platform_hsvm_jit_hlt
};



int platform_hsvm_jit_hlt (struct jit * jit, struct varstore * varstore) {
    size_t offset;
    int error = varstore_offset(varstore, "halt_code", 8, &offset);
    if (error) {
        btlog("[%s] error finding halt_code");
        return PLATFORM_ERROR;
    }

    uint8_t * data_buf = (uint8_t *) varstore_data_buf(varstore);

    /* handle the HLT instruction */
    if (data_buf[offset] == 0)
        return PLATFORM_STOP;

    /* handle the IN instruction */
    if (data_buf[offset] == 1) {
        varstore_offset(varstore, "in_reg", 8, &offset);
        unsigned int reg_code = data_buf[offset];
        offset = varstore_offset_create(varstore, hsvm_reg_string(reg_code), 16);
        uint8_t r;
        if (read(0, &r, 1) != 1) {
            btlog("[%s] error reading to in reg", __func__);
            return PLATFORM_ERROR;
        }
        *((uint16_t *) &(data_buf[offset])) = r;
        return PLATFORM_HANDLED;
    }

    /* handle the OUT instruction */
    if (data_buf[offset] == 2) {
        varstore_offset(varstore, "out_reg", 8, &offset);
        unsigned int reg_code = data_buf[offset];
        varstore_offset(varstore, hsvm_reg_string(reg_code), 16, &offset);
        uint8_t r = *((uint16_t *) &(data_buf[offset]));
        if (write(1, &r, 1) != 1) {
            btlog("[%s] error writing reg out", __func__);
            return PLATFORM_ERROR;
        }
        return PLATFORM_HANDLED;
    }

    /* unhandled halt code */
    btlog("[%s] unhandled halt code", __func__);
    return PLATFORM_ERROR;
}
