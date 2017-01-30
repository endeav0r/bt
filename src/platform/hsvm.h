#ifndef platform_hsvm_HEADER
#define platform_hsvm_HEADER

#include "platform/platform.h"

extern const struct platform platform_hsvm;

int platform_hsvm_jit_hlt (struct jit * jit, struct varstore * varstore);
struct list * platform_hsvm_hlt_tainted_bopers (struct varstore * varstore);
struct list * platform_hsvm_hlt_tainted_addresses (struct varstore * varstore);

#endif
