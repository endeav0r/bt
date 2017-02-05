#ifndef hooks_HEADER
#define hooks_HEADER

#include "bt/jit.h"
#include "container/list.h"
#include "container/memmap.h"
#include "container/varstore.h"
#include "object.h"

#include <stdarg.h>

enum {
    HOOK_JIT_STARTUP,
    HOOK_JIT_TRANSLATE,
    HOOK_JIT_CLEANUP
};

struct hooks_api {
    /* Called after jit, varstore and memmap are all initialized */
    int (* jit_startup) (struct jit * jit,
                         struct varstore * varstore,
                         struct memmap * memmap);

    /*
    * Called each time the jit has to translate a block, after the block is
    * translated, before the translated block is assembled.
    */
    int (* jit_translate) (struct jit * jit,
                           struct varstore * varstore,
                           struct memmap * memmap,
                           struct list * binslist);

    /*
    * Called when the jit is complete, immediately before the jit, varstore,
    * and memmap are all cleaned up.
    */
    int (* jit_cleanup) (struct jit * jit,
                         struct varstore * varstore,
                         struct memmap * memmap);
};


struct hook {
    struct object_header oh;
    const struct hooks_api * hooks_api;
};

struct hook * hook_create (const struct hooks_api * hooks_api);
void          hook_delete (struct hook * hook);
struct hook * hook_copy   (const struct hook * hook);


struct hooks {
    struct object_header oh;
    struct list * hooks;
};

struct hooks * hooks_create ();
void           hooks_delete (struct hooks * hooks);
struct hooks * hooks_copy   (const struct hooks * hooks);

int hooks_append (struct hooks * hooks, const struct hooks_api * hooks_api);

int hooks_call (struct hooks * hooks, int hook_type, ...);
int hooks_vcall (struct hooks * hooks, int hook_type, va_list args);

void global_hooks_init ();
int  global_hooks_append (const struct hooks_api * hooks_api);
int  global_hooks_call (int hook_type, ...);
void global_hooks_cleanup ();

#endif
