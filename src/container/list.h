#ifndef list_HEADER
#define list_HEADER

#include "object.h"

struct list_it {
    void * obj;
    struct list_it * next;
    struct list_it * prev;
};


struct list {
    const struct object * object;
    struct list_it * front;
    struct list_it * back;
};


struct list * list_create ();
void          list_delete (struct list * list);
struct list * list_copy   (const struct list * list);

void   list_append       (struct list * list, const void * obj);
void   list_append_      (struct list * list, void * obj);
void   list_append_list  (struct list * dst, const struct list * src);
void   list_prepend      (struct list * list, const void * obj);
void   list_prepend_     (struct list * list, void * obj);
void * list_front        (struct list * list);
void * list_back         (struct list * list);
void   list_pop_front    (struct list * list);
void   list_pop_back     (struct list * list);

struct list_it * list_it        (struct list * list);
void *           list_it_data   (struct list_it * it);
struct list_it * list_it_next   (struct list_it * it);
struct list_it * list_it_remove (struct list * list, struct list_it * it);

#endif
