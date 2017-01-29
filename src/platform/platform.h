#ifndef platform_HEADER
#define platform_HEADER

#include "bt/jit.h"
#include "container/varstore.h"

/* These are the return codes for platform functions */
enum {
    /*
    * Some error condition was encountered for this platform. Stop executing.
    */
    PLATFORM_ERROR,

    /*
    * Stop executing normally. The program has naturally terminated.
    */
    PLATFORM_STOP,

    /*
    * The halt code was handled. Continue operation.
    */
    PLATFORM_HANDLED,

    /*
    * Continue operation.
    * This most likely means the halt code was not handled, but you should
    * continue executing regardless.
    */
    PLATFORM_CONTINUE
};

struct platform {
    int (* jit_hlt) (struct jit * jit, struct varstore * varstore);
};

#endif
