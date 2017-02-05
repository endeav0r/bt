#ifndef platform_HEADER
#define platform_HEADER

#include "bt/jit.h"
#include "container/list.h"
#include "container/varstore.h"

/* These are the return codes for platform functions */
enum {
    PLATFORM_OK,

    /*
    * Some error condition was encountered for this platform. Stop executing.
    */
    PLATFORM_ERROR,

    /*
    * A non-fatal error has occured, such as an unhandled hlt_tainted_bopers
    * condition.
    */
    PLATFORM_NONFATAL,

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

struct jit;

struct platform {
    /* Updates the program state, using varstore to access variables and the
       memmap. Returns one of the status codes above. */
    int (* jit_hlt) (struct jit * jit, struct varstore * varstore);

    /**
    * Takes a program state when a "hlt" instruction has been reached, and
    * returns a list of boper which identify tainted variables. If no bopers
    * would be tainted by this hlt instruction, NULL is returned. Additionally,
    * if bopers may be tainted, but no bopers are actually tainted, an empty
    * list may be returned.
    * @param varstore The varstore at the time of the hlt instruction.
    * @return a list of struct boper indicating which variables may be tainted,
    *         or NULL.
    */
    struct list * (* hlt_tainted_bopers) (struct varstore * varstore);

    /**
    * Takes a program state when a "hlt" instruction has been reached, and
    * returns a list of uint64 which identify tainted memory addresses. If no
    * memory addresses would be tainted by this hlt instruction, NULL is
    * returned. Additionally, if memory addresses may be tainted, but no memory
    * addresses are actually tainted, an empty list may be returned.
    * @param varstore The varstore at the time of the hlt instruction.
    * @return a list of struct uint64 indicating which memory addresses may be
    *         tainted, or NULL.
    */
    struct list * (* hlt_tainted_addresses) (struct varstore * varstore);
};

#endif
