/*******************************************************************************
* This is a taint-tracer for binary toolkit. My goal is to thoroughly document
* this taint tracer, and make it available as an example of how to write plugins
* for bt.
*
* This taint tracer will work by maintaining two trees, one of variables, and
* one of memory addresses. If a variable or memory address is present in the
* tree, that means that variable/address is tainted. The absense of a variable
* or memory address indicates absence of taint.
*
* We will then hook the jit translation process and insert special hook
* instructions as necessary.
*******************************************************************************/

#include "btlog.h"
#include "bt/bins.h"
#include "bt/jit.h"
#include "container/list.h"
#include "container/memmap.h"
#include "container/tree.h"
#include "container/varstore.h"
#include "hooks/hooks.h"

#include <stdlib.h>
#include <string.h>

/*******************************************************************************
* struct tt_var
* This is a BT object used to track the presence/absence of taint in variables.
*******************************************************************************/

struct tt_var {
    struct object_header oh;
    const char * identifier;
};

struct tt_var * tt_var_create (const char * identifier);
void            tt_var_delete (struct tt_var * ttv);
struct tt_var * tt_var_copy   (const struct tt_var * ttv);
int             tt_var_cmp    (const struct tt_var * lhs,
                               const struct tt_var * rhs);

const struct object_vtable tt_var_vtable = {
    (void (*) (void *)) tt_var_delete,
    (void * (*) (const void *)) tt_var_copy,
    (int (*) (const void *, const void *)) tt_var_cmp
};


struct tt_var * tt_var_create (const char * identifier) {
    struct tt_var * ttv = malloc(sizeof(struct ttv_var));
    object_init(ttv, &tt_var_vtable);
    ttv->identifier = strdup(identifier);
    return ttv;
}


void ttv_var_delete (struct tt_var * ttv) {
    free(ttv->identifier);
    free(ttv);
}


struct tt_var * tt_var_copy (const struct tt_var * ttv) {
    return tt_var_create(ttv->identifier);
}


int tt_var_cmp (const struct tt_var * lhs, const struct tt_var * rhs) {
    return strcmp(lhs->identifier, rhs->identifier);
}


/*******************************************************************************
* struct tt_mem
* This is a BT object used to track the presence/absence of taint in memory.
*******************************************************************************/


struct tt_mem {
    struct object_header oh;
    uint64_t address;
};

struct tt_mem * tt_mem_create (uint64_t address);
void            tt_mem_delete (struct tt_mem * ttm);
struct tt_mem * tt_mem_copy   (const sturct tt_mem * ttm);
int             tt_mem_cmp    (const struct tt_mem * lhs,
                               const struct tt_mem * rhs);


const struct object_vtable tt_mem_vtable = {
    (void (*) (void *)) tt_mem_delete,
    (void * (*) (const void *)) tt_mem_copy,
    (int (*) (const void *, const void *)) tt_mem_cmp
};


struct tt_mem * tt_mem_create (const char * identifier) {
    struct tt_mem * ttm = malloc(sizeof(struct tt_mem));
    object_init(ttm, &tt_mem_vtable);
    ttm->address = address;
    return ttm;
}


void tt_mem_delete (struct tt_mem * ttm) {
    free(ttm);
}


struct tt_mem * tt_mem_copy (const struct tt_mem * ttm) {
    return tt_mem_create(ttm->address);
}


int tt_mem_cmp (const struct tt_mem * lhs, const struct tt_mem * rhs) {
    if (lhs->address < rhs->address)
        return -1;
    else if (lhs->address > rhs->address)
        return 1;
    return 0;
}


/*******************************************************************************
* struct tt_bins
* This is a BT object used to track bins instructions. This allows us to set a
* varstore variable in a bins block associated with a tt_bins, then set a hook
* instruction. Once the hook executes, we can use the varstore variable to
* retrieve the tt_bins, and therefor the bins associated with that hook.
*******************************************************************************/


struct tt_bins {
    struct object_header oh;
    uint64_t identifier;
    struct bins * bins;
};

struct tt_bins * tt_bins_create (uint64_t identifier, struct bins * bins);
void             tt_bins_delete (struct tt_bins * ttb);
struct tt_bins * tt_bins_copy   (const sturct tt_bins * ttb);
int              tt_bins_cmp    (const struct tt_bins * lhs,
                                 const struct tt_bins * rhs);


const struct object_vtable tt_bins_vtable = {
    (void (*) (void *)) tt_bins_delete,
    (void * (*) (const void *)) tt_bins_copy,
    (int (*) (const void *, const void *)) tt_bins_cmp
};


struct tt_bins * tt_bins_create (uint64_t identifer, struct bins * bins) {
    struct tt_bins * ttb = malloc(sizeof(struct tt_bins));
    object_init(ttb, &tt_bins_vtable);
    ttb->identifier = identifier;
    ttb->bins = OCOPY(bins);
    return ttb;
}


void tt_bins_delete (struct tt_bins * ttb) {
    ODEL(ttb);
    free(ttb);
}


struct tt_bins * tt_bins_copy (const struct tt_bins * ttb) {
    return tt_bins_create(ttb->identifier, ttb->bins);
}


int tt_bins_cmp (const struct tt_bins * lhs, const struct tt_bins * rhs) {
    if (lhs->identifier < rhs->identifier)
        return -1;
    else if (lhs->identifier > rhs->identifier)
        return 1;
    return 0;
}


/*******************************************************************************
* These variables hold the plugin's global state
*******************************************************************************/

struct tt {
    /* A tree of struct tt_var */
    struct tree * variables;

    /* A tree of struct tt_mem */
    struct tree * addresses;

    /* A tree of struct tt_bins */
    struct tree * bins;

    /*
    * We increment this each time we need to create a new tt_bins, and use it
    * to correlate hooks to tt_bins objects.
    */
    uint64_t tt_bins_identifier;

    /*
    * A list of traced bins instructions. Some instructions/operands will be
    * tagged with additional information.
    */
    struct list * trace;
};

struct tt * tt = NULL;

/*******************************************************************************
* Initialization and cleanup routines for the plugin
*******************************************************************************/

int plugin_initialize () {
    tt = malloc(sizeof(struct tt));
    tt->variables = tree_create();
    tt->addresses = tree_create();
    tt->bins = tree_create();
    tt->tt_bins_identifier = 0;
    tt->trace = list_create();
    return 0;
}


int plugin_cleanup () {
    struct list_it * it;
    for (it = list_it(tt->trace); it != NULL; it = list_it_next) {
        struct bins * bins = list_it_data(it);
        char * bins_str = bins_string(bins);
        btlog("[tainttrace] %s", bins_str);
        free(bins_str);
    }
    ODEL(tt->variables);
    ODEL(tt->addresses);
    ODEL(tt->bins);
    ODEL(tt->trace);
    free(tt);
    return 0;
}

int taint_trace_jit_translate (struct jit * jit,
                               struct varstore * varstore,
                               struct memmap * memmap,
                               struct list * binslist);


const struct hooks_api taint_trace_hooks = {
    taint_trace_jit_translate
};


const struct hooks_api * plugin_hooks () {
    return &taint_trace_hooks;
}

/*******************************************************************************
* Taint tracer helper functions
*******************************************************************************/

/*
* returns
*   1 if boper is a tainted variable
*   0 if boper is not a tainted variable.
*/
int tt_boper_tainted (const struct boper * boper) {
    if (boper_type(boper) != BOPER_VARIABLE)
        return 0;

    int tainted = 0;
    struct tt_var * ttv = tt_var_create(boper->identifier);
    if (tree_fetch(tt->variables, ttv))
        tainted = 1;
    ODEL(ttv);

    return tainted;
}


int tt_boper_taint (const struct boper * boper) {
    struct tt_var * ttv = tt_var_create(boper_identifier(boper));
    if (tree_insert_(tt->variables, ttv))
        ODEL(ttv);
    return 0;
}


int tt_boper_untaint (const struct boper * boper) {
    struct tt_var * ttv = tt_var_create(boper_identifier(boper));
    tree_remove(tt->variables, ttv);
    ODEL(ttv);
    return 0;
}


int tt_address_tainted (uint64_t address) {
    struct tt_mem * ttm = tt_mem_create(address);
    int tainted = 0;
    if (tree_fetch(tt->addresses, ttm))
        tainted = 1;
    ODEL(ttm);
    return tainted;
}


int tt_address_taint (uint64_t address) {
    struct tt_mem * ttm = tt_mem_create(address);
    if (tree_insert_(tt->addresses, ttm))
        ODEL(ttm);
    return 0;
}


int tt_address_untaint (uint64_t address) {
    struct tt_mem * ttm = tt_mem_create(address);
    tree_remove(tt->addresses, ttm);
    ODEL(ttm);
    return 0;
}


int tt_boper_value (struct varstore * varstore,
                    const struct boper * boper,
                    uint64_t * address) {
    switch (boper_bits(boper)) {
    case 8 : {
        uint8_t u8;
        if (varstore_value(varstore, boper_identifier(boper), 8, &u8))
            return -1;
        *address = u8;
        return 0;
    }
    case 16 : {
        uint16_t u16;
        if (varstore_value(varstore, boper_identifier(boper), 16, &u16))
            return -1;
        *address = u16;
        return 0;
    }
    case 32 : {
        uint32_t u32;
        if (varstore_value(varstore, boper_identifier(boper), 32, &u32))
            return -1;
        *address = u32;
        return 0;
    }
    case 64 : {
        if (varstore_value(varstore, boper_identifier(boper), 64, address))
            return -1;
        return 0;
    }
    }
    return -1;
}


struct tt_bins * tt_hook_get_tt_bins (struct varstore * varstore) {
    /* Get the instruction associated with this hook. */
    uint64_t tt_bins_identifier;
    int error = varstore_value(varstore,
                               "tt_bins_identifier",
                               64,
                               &tt_bins_identifier);
    if (error) {
        btlog("[-] Could not find tt_bins_identifier");
        return NULL;
    }

    /* Retrieve the instruction associated with this hook. */
    struct tt_bins tt_bins_needle;
    object_init(&tt_bins_needle, &tt_bins_vtable);
    tt_bins_needle->identiifier = tt_bins_identifier;
    struct tt_bins * tt_bins = tree_fetch(tt->bins, &tt_bins_needle);
    if (bins == NULL) {
        btlog("[-] Could not find tt_bins for 0x%llx",
              (unsigned long long) tt_bins_identifier);
        return NULL;
    }

    return tt_bins;
}


/*******************************************************************************
* The hook functions
*******************************************************************************/

void tt_arithmetic_hook (struct varstore * varstore) {
    /* get the tt_bins, and bins, for this instruction */
    struct tt_bins * tt_bins = tt_hook_get_tt_bins(varstore);
    if (tt_bins == NULL)
        return;
    struct bins * bins = tt_bins->bins;

    /* We now update taint as needed. */
    int tainted = 0;
    switch (bins->op) {
    /* These are the instructions which we will always propogate taint for. */
    case BOP_ADD :
    case BOP_UMUL :
    case BOP_UDIV :
    case BOP_UMOD :
    case BOP_OR :
    case BOP_SHL :
    case BOP_SHR : {
        if (    (tt_boper_tainted(bins->oper[1]))
             || (tt_boper_tainted(bins->oper[2])))
            tainted = 1;
        break;
    }
    /* These instructions remove taint if lhs == rhs, and otherwise propogate
       taint. */
    case BOP_SUB :
    case BOP_XOR : {
        if (boper_cmp(bins->oper[1], bins->oper[2]) == 0)
            break;
        if (    (tt_boper_tainted(bins->oper[1]))
             || (tt_boper_tainted(bins->oper[2])))
             tainted = 1;
        break;
    }
    /* And removes taint if bins->oper[1] or bins->oper[2] is 0, and propogates
       taint otherwise. */
    case BOP_AND :
        if (    (boper_type(bins->oper[1]) == BOPER_CONSTANT)
             && (boper_value(bins->oper[1]) == 0))
            break;
        if (    (boper_type(bins->oper[2]) == BOPER_CONSTANT)
             && (boper_value(bins->oper[2]) == 0))
            break;
        if (    (tt_boper_tainted(bins->oper[1]))
             || (tt_boper_tainted(bins->oper[2])))
             tainted = 1;
        break;
    }

    /*
    * We now propogate taint as necessary, and log instructions.
    * We log all instructions which either propogate taint, or cause a tainted
    * dst operand to become untainted.
    * We do not log instructions which do not propogate taint AND do not modify
    * the taintedness of operands.
    */

    /* Log first. */
    if ((tainted) || (tt_boper_tainted(bins->oper[0]))) {
        /* Create a copy of this bins. */
        struct bins * logbins = OCOPY(bins);
        /* Get the tags for this logins. We are going to add supplementary
           information */
        struct tags * tags = OTAGS(logbins);
        /* Get the value for oper[1] if it is a variable */
        if (boper_type(bins->oper[1]) == BOPER_VARIABLE) {
            uint64_t value;
            int error = tt_boper_value(varstore, bins->oper[1], &value);
            if (error) {
                btlog("[-] Could not get boper_1_value for %s",
                      boper_identifier(bins->oper[1]));
            }
            else
                tags_set_uint64(tags, "oper_1_value", value);
        }
        /* Get the value for oper[2] if it is a variable */
        if (boper_type(bins->oper[2]) == BOPER_VARIABLE) {
            uint64_t value;
            int error = tt_boper_value(varstore, bins->oper[2], &value);
            if (error) {
                btlog("[-] Could not get boper_2_value for %s",
                      boper_identifier(bins->oper[2]));
            }
            else
                tags_set_uint64(tags, "oper_2_value", value);
        }
        list_append_(tt->trace, logbins);
    }

    /* Now propogate taint */
    if (tainted)
        tt_boper_taint(bins->oper[0]);
    else
        tt_boper_untaint(biner->oper[0]);
}


void tt_comparison_hook (struct varstore * varstore) {
    /* get the tt_bins, and bins, for this instruction */
    struct tt_bins * tt_bins = tt_hook_get_tt_bins(varstore);
    if (tt_bins == NULL)
        return;
    struct bins * bins = tt_bins->bins;

    /*
    * The result of a comparison is tainted if the lhs or rhs of a comparison
    * is tainted.
    */
    if (    (tt_boper_tainted(bins->oper[1]))
         || (tt_boper_tainted(bins->oper[2])))
        tt_boper_taint(bins->oper[0]);
    else
        tt_boper_untaint(bins->oper[1]);
}


void tt_extension_hook (struct varstore * varstore) {
    /* get the tt_bins, and bins, for this instruction */
    struct tt_bins * tt_bins = tt_hook_get_tt_bins(varstore);
    if (tt_bins == NULL)
        return;
    struct bins * bins = tt_bins->bins;

    /*
    * If the variable being extended or truncated is tainted, then the result
    * is tainted as well.
    */
    if (    (tt_boper_tainted(bins->oper[1]))
         || (tt_boper_tainted(bins->oper[2])))
        tt_boper_taint(bins->oper[0]);
    else
        tt_boper_untaint(bins->oper[1]);
}


void tt_store_hook (struct varstore * varstore) {
    /* get the tt_bins, and bins, for this instruction */
    struct tt_bins * tt_bins = tt_hook_get_tt_bins(varstore);
    if (tt_bins == NULL)
        return;
    struct bins * bins = tt_bins->bins;

    /*
    * Get the address this store writes too.
    */
    uint64_t address;
    if (boper_type(bins->oper[0]) == BOPER_CONSTANT)
        address = boper_value(bins->oper[0]);
    else if (tt_boper_value(varstore, bins->oper[0], &address)) {
        btlog("[-] error fetching address for store for %s",
              boper_identifier(bins->oper[0]));
        return;
    }

    /*
    * If the value being written to memory is tainted, then we taint that
    * memory address. Otherwise, we ensure address is untainted.
    */
    if (tt_boper_tainted(bins->oper[1]))
        tt_address_taint(address);
    else
        tt_address_untaint(address);
}


void tt_load_hook (struct varstore * varstore) {
    /* get the tt_bins, and bins, for this instruction */
    struct tt_bins * tt_bins = tt_hook_get_tt_bins(varstore);
    if (tt_bins == NULL)
        return;
    struct bins * bins = tt_bins->bins;

    /*
    * Get the address we are loading from.
    */
    uint64_t address;
    if (boper_type(bins->oper[1]) == BOPER_CONSTANT)
        address = boper_value(bins->oper[1]);
    else if (tt_boper_value(varstore, bins->oper[1], &address)) {
        btlog("[-] error fetching address for store for %s",
              boper_identifier(bins->oper[1]));
        return;
    }

    /*
    * If this address is tainted, we propogate this taint to the variable of
    * the load instruction.
    */
    if (tt_address_tainted(address))
        tt_boper_taint(bins->oper[0]);
    else
        tt_boper_untaint(bins->oper[1]);
}


/*******************************************************************************
* This begins the real meat of the plugin
*******************************************************************************/
int taint_trace_jit_translate (struct jit * jit,
                               struct varstore * varstore,
                               struct memmap * memmap
                               struct list * binslist) {
    struct list_it * it = NULL;
    for (it = list_it(binslist); it != NULL; it = list_it_next) {
        struct bins * bins = list_it_data(it);
        void (* function_ptr) (void *) = NULL;
        switch (bins->op) {
        /* We hook almost, but not all, bins instruction types. */
        case BOP_ADD :
        case BOP_SUB :
        case BOP_UMUL :
        case BOP_UDIV :
        case BOP_UMOD :
        case BOP_AND :
        case BOP_OR  :
        case BOP_XOR :
        case BOP_SHL :
        case BOP_SHR :
            function_ptr = (void (*) (void *)) tt_arithmetic_hook;
        case BOP_CMPEQ :
        case BOP_CMPLTU :
        case BOP_CMPLTS :
        case BOP_CMPLEU :
        case BOP_CMPLES :
            if (function_ptr == NULL)
                function_ptr = (void (*) (void *)) tt_comparison_hook;
        case BOP_SEXT :
        case BOP_ZEXT :
        case BOP_TRUN :
            if (function_ptr == NULL)
                function_ptr = (void (*) (void *)) tt_extension_hook;
        case BOP_STORE :
            if (function_ptr == NULL)
                function_ptr = (void (*) (void *)) tt_store_hook;
        case BOP_LOAD : {
            if (function_ptr == NULL)
                function_ptr = (void (*) (void *)) tt_load_hook;
            /*
            * Create a tt_bins and add it to tt->bins to track this bins.
            */
            struct tt_bins * ttb = tt_bins_create(tt->identifier++, bins);
            tree_insert(tt->bins, ttb);
            /*
            * Create a bins which sets a variable "tt_bins_identifier" to the
            * identifier for this tt_bins.
            */
            struct bins * ttb_bins;
            ttb_bins = bins_or_(boper_variable(64, "tt_bins_identifier"),
                                boper_constant(64, ttb->identifier),
                                boper_constant(64, 0));
            /*
            * Inject this instruction into our bins list and advance our
            * iterator.
            */
            list_it_append_(binslist, it, ttb_bins);
            it = list_it_next(it);
            /*
            * Inject a hook into our list bins which calls tt_arithmetic_hook,
            * and advance the iterator.
            */
            struct bins * hook_bins;
            hook_bins = bins_hook(function_ptr);
            list_it_append(binslist, it, hook_bins);
            it = list_it_next;
            break;
        }
        }
    }
}
