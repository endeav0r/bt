#ifndef object_HEADER
#define object_HEADER

struct object_vtable {
    void (* delete) (void *);
    void * (* copy) (const void *);
    int (* cmp) (const void *, const void *);
};

struct object_header {
    const struct object_vtable * vtable;
    struct tags * tags;
};

void   object_init (void * obj, const struct object_vtable * vtable);
void   object_delete (void * obj);
void * object_copy (const void * obj);
int    object_cmp (const void * lhs, const void * rhs);
struct tags * object_tags (void * obj);

#define ODEL  object_delete
#define OCOPY object_copy
#define OCMP  object_cmp
#define OTAGS object_tags

#endif
