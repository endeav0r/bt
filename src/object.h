#ifndef object_HEADER
#define object_HEADER

struct object_header {
    void (* delete) (void *);
    void * (* copy) (const void *);
    int (* cmp) (const void *, const void *);
};

void   object_delete (void * obj);
void * object_copy (const void * obj);
int    object_cmp (const void * lhs, const void * rhs);

#define ODEL  object_delete
#define OCOPY object_copy
#define OCMP  object_cmp

#endif