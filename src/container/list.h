#ifndef list_HEADER
#define list_HEADER

#include "object.h"

struct list_it {
    void * obj;
    struct list_it * next;
    struct list_it * prev;
};


struct list {
    struct object_header oh;
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

unsigned int list_length (const struct list * list);

/**
* Takes an iterator to the first element of a slice, the last element of a
* slice, and returns a new list from first to last inclusive.
* @param list the list we will create our sliced list from.
* @param first An iterator to the first element, or NULL if we should begin from
*        the beginning of the list.
* @param last An iterator to the last element, or NULL if we should go to the
*        last element of the list.
* @return A new list, with a deep copy of all elements from first to last.
*/
struct list * list_slice (
    struct list * list,
    struct list_it * first,
    struct list_it * last
);

struct list_it * list_it        (struct list * list);
void *           list_it_data   (struct list_it * it);
struct list_it * list_it_next   (struct list_it * it);
struct list_it * list_it_remove (struct list * list, struct list_it * it);

/**
* Appends data so that it immediately follows the given iterator.
* @param list Pointer to a list.
* @param it Pointer to an iterator in list..
* @param data The data to append.
* @return 0 on success, non-zero on failure.
*/
int list_it_append_ (struct list * list, struct list_it * it, void * data);
int list_it_append  (struct list * list, struct list_it * it, const void * data);

int list_it_prepend_ (struct list * list, struct list_it * it, void * data);
int list_it_prepend  (struct list * list, struct list_it * it, const void * data);

#endif
