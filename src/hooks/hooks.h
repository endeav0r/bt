#ifndef hooks_HEADER
#define hooks_HEADER

#include "bt/jit.h"
#include "container/list.h"
#include "container/memmap.h"
#include "container/varstore.h"
#include "object.h"


struct hooks_api {
    int (* jit_translate) (struct jit * jit,
                           struct varstore * varstore,
                           struct memmap * memmap,
                           struct list * binslist);
};


struct hook {
    struct object_header oh;
    union {
        void * (* f) ();
        void * (* f1) (void *);
        void * (* f2) (void *, void *);
        void * (* f3) (void *, void *, void *);
        void * (* f4) (void *, void *, void *, void *);
    };
};


struct hooks {
    struct object_header oh;
    struct list * hooks;
};

#endif
