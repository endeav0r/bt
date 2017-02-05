#include "hooks.h"

#include <stdlib.h>

const struct object_vtable hook_vtable = {
    (void (*) (void *)) hook_delete,
    (void * (*) (const void *)) hook_copy,
    NULL
};


struct hook * hook_create (const struct hooks_api * hooks_api) {
    struct hook * hook = malloc(sizeof(struct hook));
    object_init(hook, &hook_vtable);
    hook->hooks_api = hooks_api;
    return hook;
}


void hook_delete (struct hook * hook) {
    free(hook);
}


struct hook * hook_copy (const struct hook * hook) {
    return hook_create(hook->hooks_api);
}


const struct object_vtable hooks_vtable = {
    (void (*) (void *)) hooks_delete,
    (void * (*) (const void *)) hooks_copy,
    NULL
};


struct hooks * hooks_create () {
    struct hooks * hooks = malloc(sizeof(struct hooks));
    object_init(hooks, &hooks_vtable);
    hooks->hooks = list_create();
    return hooks;
}


void hooks_delete (struct hooks * hooks) {
    ODEL(hooks->hooks);
    free(hooks);
}


struct hooks * hooks_copy (const struct hooks * hooks) {
    struct hooks * copy = hooks_create();
    ODEL(copy->hooks);
    copy->hooks = OCOPY(hooks->hooks);
    return copy;
}


int hooks_append (struct hooks * hooks, const struct hooks_api * hooks_api) {
    list_append_(hooks->hooks, hook_create(hooks_api));
    return 0;
}


int hooks_call (struct hooks * hooks, int hook_type, ...) {
    va_list args;
    va_start(args, hook_type);
    int result = hooks_vcall(hooks, hook_type, args);
    va_end(args);
    return result;
}


int hooks_vcall (struct hooks * hooks, int hook_type, va_list args) {
    if (hook_type == HOOK_JIT_STARTUP) {
        struct jit * jit = va_arg(args, struct jit *);
        struct varstore * varstore = va_arg(args, struct varstore *);
        struct memmap * memmap = va_arg(args, struct memmap *);

        struct list_it * it;
        for (it = list_it(hooks->hooks); it != NULL; it = list_it_next(it)) {
            struct hook * hook = list_it_data(it);
            if (hook->hooks_api->jit_startup == NULL)
                continue;
            hook->hooks_api->jit_startup(jit, varstore, memmap);
        }
    }
    else if (hook_type == HOOK_JIT_TRANSLATE) {
        struct jit * jit = va_arg(args, struct jit *);
        struct varstore * varstore = va_arg(args, struct varstore *);
        struct memmap * memmap = va_arg(args, struct memmap *);
        struct list * binslist = va_arg(args, struct list *);

        struct list_it * it;
        for (it = list_it(hooks->hooks); it != NULL; it = list_it_next(it)) {
            struct hook * hook = list_it_data(it);
            if (hook->hooks_api->jit_translate == NULL)
                continue;
            hook->hooks_api->jit_translate(jit, varstore, memmap, binslist);
        }
    }
    else if (hook_type == HOOK_JIT_CLEANUP) {
        struct jit * jit = va_arg(args, struct jit *);
        struct varstore * varstore = va_arg(args, struct varstore *);
        struct memmap * memmap = va_arg(args, struct memmap *);

        struct list_it * it;
        for (it = list_it(hooks->hooks); it != NULL; it = list_it_next(it)) {
            struct hook * hook = list_it_data(it);
            if (hook->hooks_api->jit_cleanup == NULL)
                continue;
            hook->hooks_api->jit_cleanup(jit, varstore, memmap);
        }
    }
    else
        return -1;
    return 0;
}


struct hooks * global_hooks = NULL;

void global_hooks_init () {
    global_hooks = hooks_create();
}

int global_hooks_append (const struct hooks_api * hooks_api) {
    return hooks_append(global_hooks, hooks_api);
}

int global_hooks_call (int hook_type, ...) {
    va_list args;
    va_start(args, hook_type);
    int result = hooks_vcall(global_hooks, hook_type, args);
    va_end(args);

    return result;
}

void global_hooks_cleanup () {
    ODEL(global_hooks);
}
