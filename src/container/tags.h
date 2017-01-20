#ifndef tags_HEADER
#define tags_HEADER

#include "object.h"
#include "tree.h"

#include <inttypes.h>

enum {
    TAG_UINT64,
    TAG_STRING,
    TAG_OBJECT
};

struct tag {
    struct object_header oh;
    char * name;
    int type;
    union {
        uint64_t uint64;
        char * string;
        void * object;
    };
};


struct tag * tag_create_uint64  (const char * name, uint64_t uint64);
struct tag * tag_create_string  (const char * name, const char * string);
struct tag * tag_create_object_ (const char * name, void * obj);
struct tag * tag_create_object  (const char * name, const void * obj);
void         tag_delete        (struct tag * tag);
struct tag * tag_copy          (const struct tag * tag);
int          tag_cmp           (const struct tag * lhs, const struct tag * rhs);

int          tag_type   (const struct tag * tag);
uint64_t     tag_uint64 (const struct tag * tag);
const char * tag_string (const struct tag * tag);
void *       tag_object (struct tag * tag);


struct tags {
    struct object_header oh;
    struct tree * tags;
};


struct tags * tags_create ();
void          tags_delete (struct tags * tags);
struct tags * tags_copy   (const struct tags * tags);

int          tags_exists (const struct tags * tags, const char * name);
struct tag * tags_tag    (struct tags * tags, const char * name);

int tags_set_uint64 (struct tags * tags,
                     const char * name,
                     uint64_t uint64);
int tags_set_string (struct tags * tags,
                     const char * name,
                     const char * string);
int tags_set_object_ (struct tags * tags,
                      const char * name,
                      void * obj);
int tags_set_object (struct tags * tags,
                     const char * name,
                     const void * obj);

#endif
