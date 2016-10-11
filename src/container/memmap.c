#include "memmap.h"

#include <string.h>

const struct object memmap_page_object = {
    (void (*) (void *))                    memmap_page_delete,
    (void * (*) (const void *))            memmap_page_copy,
    (int (*) (const void *, const void *)) memmap_page_cmp
};


struct memmap_page * memmap_page_create (uint64_t address,
                                         size_t size,
                                         unsigned int permissions) {
    struct memmap_page * memmap_page = malloc(sizeof(struct memmap_page));

    memmap_page->object  = &memmap_page_object;
    memmap_page->address = address;
    memmap_page->data    = malloc(size);
    memmap_page->size    = size;
    memmap_page->permissions = permissions;

    return memmap_page;
}


void memmap_page_delete (struct memmap_page * memmap_page) {
    free(memmap_page->data);
    free(memmap_page);
}


struct memmap_page * memmap_page_copy (const struct memmap_page * memmap_page) {
    struct memmap_page * copy = memmap_page_create(memmap_page->address,
                                                   memmap_page->size,
                                                   memmap_page->permissions);
    memcpy(copy->data, memmap_page->data, memmap_page->size);
    return copy;
}


int memmap_page_cmp (const struct memmap_page * lhs,
                     const struct memmap_page * rhs) {
    if (lhs->address < rhs->address)
        return -1;
    else if (lhs->address > rhs->address)
        return 1;
    return 0;
}


const struct object memmap_object = {
    (void (*) (void *)) memmap_delete,
    (void * (*) (const void *)) memmap_copy,
    NULL
};


struct memmap * memmap_create (unsigned int page_size) {
    struct memmap * memmap = malloc(sizeof(struct memmap));

    memmap->object = &memmap_object;
    memmap->tree = tree_create();
    memmap->page_size = page_size;

    return memmap;
}


void memmap_delete (struct memmap * memmap) {
    ODEL(memmap->tree);
    free(memmap);
}


struct memmap * memmap_copy (const struct memmap * memmap) {
    struct memmap * copy = memmap_create(memmap->page_size);
    ODEL(copy->tree);
    copy->tree = OCOPY(memmap->tree);
    return copy;
}


int memmap_map (struct memmap * memmap,
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
    uint64_t page_address = address & (~(memmap->page_size - 1));
    uint64_t page_offset  = address - page_address;

    // we use this page to search for pages
    struct memmap_page * needle = memmap_page_create(page_address, 0, 0);

    struct memmap_page * page = tree_fetch(memmap->tree, needle);
    // first page doesn't exist, create it
    if (page == NULL) {
        page = memmap_page_create(page_address, memmap->page_size, permissions);
        tree_insert(memmap->tree, page);
    }
    // set permissions
    page->permissions = permissions;

    // set mapped bytes for first page
    mapped_bytes = memmap->page_size - page_offset;

    // first page exists, do we have any data to copy in?
    copied_bytes = 0;
    if (buf_size > 0) {
        uint64_t copy_size = buf_size;
        if (copy_size > memmap->page_size - page_offset)
            copy_size = memmap->page_size - page_offset;
        memcpy(&(page->data[page_offset]), buf, copy_size);
        copied_bytes = copy_size;
    }

    // and now we loop
    while (mapped_bytes < size) {
        // increase page address
        page_address += memmap->page_size;
        mapped_bytes += memmap->page_size;

        // fetch this page
        needle->address = page_address;
        page = tree_fetch(memmap->tree, needle);
        if (page == NULL) {
            page = memmap_page_create(page_address, memmap->page_size, permissions);
            tree_insert(memmap->tree, page);
        }
        page->permissions = permissions;

        // copy over any data that requires copying
        if (copied_bytes < buf_size) {
            uint64_t copy_size = buf_size - copied_bytes;
            if (copy_size > memmap->page_size)
                copy_size = memmap->page_size;
            memcpy(page->data, &(buf[copied_bytes]), copy_size);
            copied_bytes += copy_size;
        }
    }

    ODEL(needle);

    return 0;
}


uint8_t memmap_byte_get (const struct memmap * memmap,
                       uint64_t address,
                       int * error) {
    struct memmap_page page;

    uint64_t page_address = address & (~(memmap->page_size - 1));
    uint64_t page_offset  = address - page_address;

    page.address = page_address;

    struct memmap_page * tree_page = tree_fetch(memmap->tree, &page);
    if (tree_page == NULL) {
        *error = 1;
        return 0;
    }

    return tree_page->data[page_offset];
}


int memmap_byte_set (struct memmap * memmap, uint64_t address, uint8_t byte) {
    struct memmap_page page;

    uint64_t page_address = address & (~(memmap->page_size - 1));
    uint64_t page_offset  = address - page_address;

    page.address = page_address;

    struct memmap_page * tree_page = tree_fetch(memmap->tree, &page);
    if (tree_page == NULL) {
        return 1;
    }

    tree_page->data[page_offset] = byte;
    return 0;
}


int memmap_get_u8 (const struct memmap * memmap, uint64_t address, uint8_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address, &error);
    return error;
}


int memmap_get_u16_be (const struct memmap * memmap,
                     uint64_t address,
                     uint16_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 1, &error);
    return error;
}


int memmap_get_u16_le (const struct memmap * memmap,
                     uint64_t address,
                     uint16_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address + 1, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address, &error);
    return error;
}


int memmap_get_u32_be (const struct memmap * memmap,
                     uint64_t address,
                     uint32_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 1, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 2, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 3, &error);
    return error;
}


int memmap_get_u32_le (const struct memmap * memmap,
                     uint64_t address,
                     uint32_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address + 3, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 2, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 1, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address, &error);
    return error;
}


int memmap_get_u64_be (const struct memmap * memmap,
                     uint64_t address,
                     uint64_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 1, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 2, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 3, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 4, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 5, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 6, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 7, &error);
    return error;
}


int memmap_get_u64_le (const struct memmap * memmap,
                     uint64_t address,
                     uint64_t * value) {
    int error = 0;
    *value = memmap_byte_get(memmap, address + 7, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 6, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 5, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 4, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 3, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 2, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address + 1, &error);
    *value <<= 8;
    *value = memmap_byte_get(memmap, address, &error);
    return error;
}


int memmap_set_u8 (struct memmap * memmap, uint64_t address, uint8_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, value);
    return error;
}


int memmap_set_u16_le (struct memmap * memmap,
                       uint64_t address,
                       uint16_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, (value >> 0) & 0xff);
    error |= memmap_byte_set(memmap, address + 1, (value >> 8) & 0xff);
    return error;
}


int memmap_set_u16_be (struct memmap * memmap,
                       uint64_t address,
                       uint16_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, (value >> 8) & 0xff);
    error |= memmap_byte_set(memmap, address + 1, (value >> 0) & 0xff);
    return error;
}


int memmap_set_u32_le (struct memmap * memmap,
                       uint64_t address,
                       uint32_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, (value >> 0) & 0xff);
    error |= memmap_byte_set(memmap, address + 1, (value >> 8) & 0xff);
    error |= memmap_byte_set(memmap, address + 2, (value >> 16) & 0xff);
    error |= memmap_byte_set(memmap, address + 3, (value >> 24) & 0xff);
    return error;
}


int memmap_set_u32_be (struct memmap * memmap,
                       uint64_t address,
                       uint32_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, (value >> 24) & 0xff);
    error |= memmap_byte_set(memmap, address + 1, (value >> 16) & 0xff);
    error |= memmap_byte_set(memmap, address + 2, (value >> 8) & 0xff);
    error |= memmap_byte_set(memmap, address + 3, (value >> 0) & 0xff);
    return error;
}


int memmap_set_u64_le (struct memmap * memmap,
                       uint64_t address,
                       uint64_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, (value >> 0) & 0xff);
    error |= memmap_byte_set(memmap, address + 1, (value >> 8) & 0xff);
    error |= memmap_byte_set(memmap, address + 2, (value >> 16) & 0xff);
    error |= memmap_byte_set(memmap, address + 3, (value >> 24) & 0xff);
    error |= memmap_byte_set(memmap, address + 4, (value >> 32) & 0xff);
    error |= memmap_byte_set(memmap, address + 5, (value >> 40) & 0xff);
    error |= memmap_byte_set(memmap, address + 6, (value >> 48) & 0xff);
    error |= memmap_byte_set(memmap, address + 7, (value >> 56) & 0xff);
    return error;
}


int memmap_set_u64_be (struct memmap * memmap,
                       uint64_t address,
                       uint64_t value) {
    int error = 0;
    error |= memmap_byte_set(memmap, address, (value >> 56) & 0xff);
    error |= memmap_byte_set(memmap, address + 1, (value >> 48) & 0xff);
    error |= memmap_byte_set(memmap, address + 2, (value >> 40) & 0xff);
    error |= memmap_byte_set(memmap, address + 3, (value >> 32) & 0xff);
    error |= memmap_byte_set(memmap, address + 4, (value >> 24) & 0xff);
    error |= memmap_byte_set(memmap, address + 5, (value >> 16) & 0xff);
    error |= memmap_byte_set(memmap, address + 6, (value >> 8) & 0xff);
    error |= memmap_byte_set(memmap, address + 7, (value >> 0) & 0xff);
    return error;
}