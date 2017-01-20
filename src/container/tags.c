#include "tags.h"

#include <stdlib.h>
#include <string.h>

const struct object_vtable tag_vtable = {
    (void (*) (void *)) tag_delete,
    (void * (*) (const void *)) tag_copy,
    (int (*) (const void *, const void *)) tag_cmp
};


struct tag * tag_create (const char * name, int type) {
    struct tag * tag = malloc(sizeof(struct tag));
    object_init(&(tag->oh), &tag_vtable);

    tag->name = strdup(name);
    tag->type = type;

    return tag;
}


struct tag * tag_create_uint64 (const char * name, uint64_t uint64) {
    struct tag * tag = tag_create(name, TAG_UINT64);
    tag->uint64 = uint64;
    return tag;
}


struct tag * tag_create_string (const char * name, const char * string) {
    struct tag * tag = tag_create(name, TAG_STRING);
    tag->string = strdup(string);
    return tag;
}


struct tag * tag_create_object_ (const char * name, void * object) {
    struct tag * tag = tag_create(name, TAG_OBJECT);
    tag->object = object;
    return tag;
}


struct tag * tag_create_object (const char * name, const void * obj) {
    return tag_create_object_(name, OCOPY(obj));
}


void tag_delete (struct tag * tag) {
    free(tag->name);
    switch (tag->type) {
    case TAG_UINT64 :
        break;
    case TAG_STRING :
        free(tag->string);
        break;
    case TAG_OBJECT :
        ODEL(tag->object);
        break;
    }
    free(tag);
}


struct tag * tag_copy (const struct tag * tag) {
    switch (tag->type) {
    case TAG_UINT64 : return tag_create_uint64(tag->name, tag->uint64);
    case TAG_STRING : return tag_create_string(tag->name, tag->string);
    case TAG_OBJECT : return tag_create_object(tag->name, tag->object);
    }
    return NULL;
}


int tag_cmp (const struct tag * lhs, const struct tag * rhs) {
    return strcmp(lhs->name, rhs->name);
}


int tag_type (const struct tag * tag) {
    return tag->type;
}


uint64_t tag_uint64 (const struct tag * tag) {
    return tag->uint64;
}


const char * tag_string (const struct tag * tag) {
    return tag->string;
}


void * tag_object (struct tag * tag) {
    return tag->object;
}



const struct object_vtable tags_vtable = {
    (void (*) (void *)) tags_delete,
    (void * (*) (const void *)) tags_copy,
    NULL
};


struct tags * tags_create () {
    struct tags * tags = malloc(sizeof(struct tags));
    object_init(&(tags->oh), &tags_vtable);
    tags->tags = tree_create();
    return tags;
}


void tags_delete (struct tags * tags) {
    ODEL(tags->tags);
    free(tags);
}


struct tags * tags_copy (const struct tags * tags) {
    struct tags * copy = tags_create();
    ODEL(copy->tags);
    copy->tags = OCOPY(tags->tags);
    return copy;
}


int tag_exists (const struct tags * tags, const char * name) {
    if (tags_tag((struct tags *) tags, name))
        return 1;
    return 0;
}


struct tag * tags_tag (struct tags * tags, const char * name) {
    struct tag * needle = tag_create_uint64(name, 0);
    struct tag * tag = tree_fetch(tags->tags, needle);
    ODEL(needle);
    return tag;
}


int tags_set_uint64 (struct tags * tags, const char * name, uint64_t uint64) {
    struct tag * tag = tag_create_uint64(name, uint64);
    /* remove this tag if it exists */
    tree_remove(tags->tags, tag);
    tree_insert_(tags->tags, tag);
    return 0;
}


int tags_set_string (struct tags * tags,
                     const char * name,
                     const char * string) {
    struct tag * tag = tag_create_string(name, string);
    /* remove this tag if it exists */
    tree_remove(tags->tags, tag);
    tree_insert_(tags->tags, tag);
    return 0;
}


int tags_set_object_ (struct tags * tags, const char * name, void * obj) {
    struct tag * tag = tag_create_object_(name, obj);
    /* remove this tag if it exists */
    tree_remove(tags->tags, tag);
    tree_insert_(tags->tags, tag);
    return 0;
}


int tags_set_object (struct tags * tags, const char * name, const void * obj) {
    return tags_set_object_(tags, name, OCOPY(obj));
}
