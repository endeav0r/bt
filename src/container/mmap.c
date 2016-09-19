#include "mmap.h"

const struct object mmap_page_object = {
    (void (*) (void *)) mmap_page_delete,
    (void * (*) (const void *)) mmap_page_copy,
    (int (*) (const void *, const void *)) mmap_page_cmp
};


struct mmap_page * mmap_page_create (uint64_t address,
                                     size_t size,
                                     unsigned int permissions) {
    struct mmap_page * mmap_page = malloc(sizeof(struct mmap_page));

    mmap_page->object = &mmap_page_object;
    mmap_page->address = address;
    mmap_page->data = malloc(size);
    mmap_page->size = size;
    mmap_page->permissions = permissions;

    return mmap_page;
}


void mmap_page_delete (struct mmap_page * mmap_page) {
    free(mmap_page->data);
    free(mmap_page);
}


struct mmap_page * mmap_page_copy (const struct mmap_page * mmap_page) {
    struct mmap_page * copy = mmap_page_create(mmap_page->address,
                                               mmap_page->size,
                                               mmap_page->persmissions);
    memcpy(copy->data, mmap_page->data, mmap_page->size);
    return copy;
}


int mmap_page_cmp (const struct mmap_page * lhs, const struct mmap_page * rhs) {
    if (lhs->address < rhs->address)
        return -1;
    else if (lhs->address > rhs->address)
        return 1;
    return 0;
}


const struct object mmap_object = {
    (void (*) (void *)) mmap_delete,
    (void * (*) (const void *)) mmap_copy,
    NULL
};


struct mmap * mmap_create (unsigned int page_size) {
    struct mmap * mmap = malloc(sizeof(struct mmap));

    mmap->object = &mmap_object;
    mmap->tree = tree_create();
    mmap->page_size = page_size;

    return mmap;
}


void mmap_destroy (struct mmap * mmap) {
    ODEL(mmap->tree);
    free(mmap);
}


struct mmap * mmap_copy (const struct mmap * mmap) {
    struct mmap * copy = mmap_create(mmap->page_size);
    ODEL(copy->tree);
    copy->tree = OCOPY(mmap->tree);
    return mmap;
}


int mmap_map (struct mmap * mmap,
              uint64_t address,
              size_t size,
              const uint8_t * buf,
              size_t buf_size,
              unsigned int permissions) {

    if (buf_size > size)
        return -1;

    size_t copied_bytes = 0; // how many bytes have we copied over
    size_t mapped_bytes = 0; // how many bytes have we mapped

    // address and offset into first page
    uint64_t page_address = address & (~(mmap->page_size - 1));
    uint64_t page_offset  = address - page_address;

    // we use this page to search for pages
    struct mmap_page * needle = mmap_page_create(page_address, 0, 0);

    struct mmap_page * page = tree_fetch(mmap->tree, needle);
    // first page doesn't exist, create it
    if (page == NULL) {
        page = mmap_page_create(page_address, mmap->page_size, permissions);
        tree_insert(mmap->tree, page);
    }
    // set permissions
    page->permissions = permissions;

    // set mapped bytes for first page
    mapped_bytes = mmap->page_size - page_offset;

    // first page exists, do we have any data to copy in?
    size_t copied_bytes = 0;
    if (buf_size > 0) {
        uint64_t copy_size = buf_size;
        if (copy_size > page_size - page_offset)
            copy_size = page_size - page_offset;
        memcpy(&(page->data[page_offset]), buf, copy_size);
        copied_bytes = copy_size;
    }

    // and now we loop
    while (mapped_bytes < size) {
        // increase page address
        page_address += mmap->page_size;
        mapped_bytes += mmap->page_size;

        // fetch this page
        needle->address = page_address;
        page = tree_fetch(mmap->tree, needle);
        if (page == NULL) {
            page = mmap_page_create(page_address, mmap->page_size, permissions);
            tree_insert(mmap->tree, page);
        }
        page->permissions = permissions;

        // copy over any data that requires copying
        if (copied_bytes < buf_size) {
            uint64_t copy_size = buf_size - copied_bytes;
            if (copy_size > mmap->page_size)
                copy_size = mmap->page_size;
            memcpy(page->data, &(buf[copied_bytes]), copy_size);
            copied_bytes += copy_size;
        }
    }

    ODEL(needle);

    return 0;
}


uint8_t mmap_byte_get (const struct mmap * mmap,
                       uint64_t address,
                       int * error) {
    struct mmap_page page;

    uint64_t page_address = address & (~(mmap->page_size - 1));
    uint64_t page_offset  = address - page_address;

    page.address = page_address;

    struct mmap_page * tree_page = tree_fetch(mmap->tree, &page);
    if (tree_page == NULL) {
        *error = 1;
        return 0;
    }

    return tree_page->data[page_offset];
}


int mmap_byte_set (struct mmap * mmap, uint64_t address, uint8_t byte) {
    struct mmap_page page;

    uint64_t page_address = address & (~(mmap->page_size - 1));
    uint64_t page_offset  = address - page_address;

    page.address = page_address;

    struct mmap_page * tree_page = tree_fetch(mmap->tree, &page);
    if (tree_page == NULL) {
        return 1;
    }

    tree_page->data[page_offset] = byte;
    return 0;
}


int mmap_get_u8 (const struct mmap * mmap, uint64_t address, uint8_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address, &error);
    return error;
}


int mmap_get_u16_be (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 1, &error);
    return error;
}


int mmap_get_u16_le (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address + 1, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address, &error);
    return error;
}


int mmap_get_u32_be (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 1, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 2, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 3, &error);
    return error;
}


int mmap_get_u32_le (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address + 3, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 2, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 1, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address, &error);
    return error;
}


int mmap_get_u64_be (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 1, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 2, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 3, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 4, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 5, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 6, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 7, &error);
    return error;
}


int mmap_get_u64_le (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t * value) {
    int error = 0;
    *value = mmap_byte_get(mmap, address + 7, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 6, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 5, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 4, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 3, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 2, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address + 1, &error);
    *value <<= 8;
    *value = mmap_byte_get(mmap, address, &error);
    return error;
}


int mmap_set_u8 (const struct mmap * mmap, uint64_t address, uint8_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, value);
    return error;
}


int mmap_set_u16_le (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, (value >> 0) & 0xff);
    error |= mmap_byte_set(mmap, address + 1, (value >> 8) & 0xff);
    return error;
}


int mmap_set_u16_be (const struct mmap * mmap,
                     uint64_t address,
                     uint16_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, (value >> 8) & 0xff);
    error |= mmap_byte_set(mmap, address + 1, (value >> 0) & 0xff);
    return error;
}


int mmap_set_u32_le (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, (value >> 0) & 0xff);
    error |= mmap_byte_set(mmap, address + 1, (value >> 8) & 0xff);
    error |= mmap_byte_set(mmap, address + 2, (value >> 16) & 0xff);
    error |= mmap_byte_set(mmap, address + 3, (value >> 24) & 0xff);
    return error;
}


int mmap_set_u32_be (const struct mmap * mmap,
                     uint64_t address,
                     uint32_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, (value >> 24) & 0xff);
    error |= mmap_byte_set(mmap, address + 1, (value >> 16) & 0xff);
    error |= mmap_byte_set(mmap, address + 2, (value >> 8) & 0xff);
    error |= mmap_byte_set(mmap, address + 3, (value >> 0) & 0xff);
    return error;
}


int mmap_set_u64_le (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, (value >> 0) & 0xff);
    error |= mmap_byte_set(mmap, address + 1, (value >> 8) & 0xff);
    error |= mmap_byte_set(mmap, address + 2, (value >> 16) & 0xff);
    error |= mmap_byte_set(mmap, address + 3, (value >> 24) & 0xff);
    error |= mmap_byte_set(mmap, address + 4, (value >> 32) & 0xff);
    error |= mmap_byte_set(mmap, address + 5, (value >> 40) & 0xff);
    error |= mmap_byte_set(mmap, address + 6, (value >> 48) & 0xff);
    error |= mmap_byte_set(mmap, address + 7, (value >> 56) & 0xff);
    return error;
}


int mmap_set_u64_be (const struct mmap * mmap,
                     uint64_t address,
                     uint64_t value) {
    int error = 0;
    error |= mmap_byte_set(mmap, address, (value >> 56) & 0xff);
    error |= mmap_byte_set(mmap, address + 1, (value >> 48) & 0xff);
    error |= mmap_byte_set(mmap, address + 2, (value >> 40) & 0xff);
    error |= mmap_byte_set(mmap, address + 3, (value >> 32) & 0xff);
    error |= mmap_byte_set(mmap, address + 4, (value >> 24) & 0xff);
    error |= mmap_byte_set(mmap, address + 5, (value >> 16) & 0xff);
    error |= mmap_byte_set(mmap, address + 6, (value >> 8) & 0xff);
    error |= mmap_byte_set(mmap, address + 7, (value >> 0) & 0xff);
    return error;
}